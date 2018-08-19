/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Data Visualization module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "QSelectionPointer3D.h"
#include "QShaderHelper3D.h"
#include "QObjectHelper3D.h"
#include "QTextureHelper3D.h"
#include "QCamera3D.h"
#include "QCamera3DPrivate.h"
#include "QUtils3D.h"

#include <QtGui/qmatrix4x4.h>

namespace QtStackedBar3DVis
{

	const GLfloat sliceUnits = 2.5;

	QSelectionPointer3D::QSelectionPointer3D(QDrawer3D *drawer)
		: QObject(0),
		m_labelShader(0),
		m_pointShader(0),
		m_labelObj(0),
		m_pointObj(0),
		m_textureHelper(0),
		m_cachedTheme(drawer->theme()),
		m_labelBackground(false),
		m_drawer(drawer),
		m_cachedScene(0)
	{
		initializeOpenGL();

		QObject::connect(m_drawer, &QDrawer3D::drawerChanged,
			this, &QSelectionPointer3D::handleDrawerChange);
	}

	QSelectionPointer3D::~QSelectionPointer3D()
	{
		delete m_labelShader;
		delete m_pointShader;
		delete m_textureHelper;
	}

	void QSelectionPointer3D::initializeOpenGL()
	{
		initializeOpenGLFunctions();

		m_textureHelper = new QTextureHelper3D();
		m_drawer->initializeOpenGL();

		initShaders();
	}

	void QSelectionPointer3D::updateScene(QScene3D *scene)
	{
		m_cachedScene = scene;
	}

	void QSelectionPointer3D::renderSelectionPointer(GLuint defaultFboHandle, bool useOrtho)
	{
		Q_UNUSED(defaultFboHandle)

			glViewport(m_mainViewPort.x(), m_mainViewPort.y(),
				m_mainViewPort.width(), m_mainViewPort.height());

		QCamera3D *camera = m_cachedScene->activeCamera();

		QMatrix4x4 itModelMatrix;

		// Get view matrix
		QMatrix4x4 viewMatrix;
		QMatrix4x4 projectionMatrix;
		GLfloat viewPortRatio = (GLfloat)m_mainViewPort.width() / (GLfloat)m_mainViewPort.height();
		if (m_cachedIsSlicingActivated) {
			GLfloat sliceUnitsScaled = sliceUnits / m_autoScaleAdjustment;
			viewMatrix.lookAt(QVector3D(0.0f, 0.0f, 1.0f), zeroVector, upVector);
			projectionMatrix.ortho(-sliceUnitsScaled * viewPortRatio, sliceUnitsScaled * viewPortRatio,
				-sliceUnitsScaled, sliceUnitsScaled,
				-1.0f, 4.0f);
		}
		else if (useOrtho) {
			viewMatrix = camera->d_ptr->viewMatrix();
			GLfloat orthoRatio = 2.0f;
			projectionMatrix.ortho(-viewPortRatio * orthoRatio, viewPortRatio * orthoRatio,
				-orthoRatio, orthoRatio,
				0.0f, 100.0f);
		}
		else {
			viewMatrix = camera->d_ptr->viewMatrix();
			projectionMatrix.perspective(45.0f, viewPortRatio, 0.1f, 100.0f);
		}

		QMatrix4x4 modelMatrix;
		QMatrix4x4 MVPMatrix;

		// Position the pointer ball
		modelMatrix.translate(m_position);

		if (!m_rotation.isIdentity()) {
			modelMatrix.rotate(m_rotation);
			itModelMatrix.rotate(m_rotation);
		}

		// Scale the point with fixed values (at this point)
		QVector3D scaleVector(0.05f, 0.05f, 0.05f);
		modelMatrix.scale(scaleVector);
		itModelMatrix.scale(scaleVector);

		MVPMatrix = projectionMatrix * viewMatrix * modelMatrix;

		QVector3D lightPos = m_cachedScene->activeLight()->position();

		//
		// Draw the point
		//
		m_pointShader->bind();
		m_pointShader->setUniformValue(m_pointShader->lightP(), lightPos);
		m_pointShader->setUniformValue(m_pointShader->view(), viewMatrix);
		m_pointShader->setUniformValue(m_pointShader->model(), modelMatrix);
		m_pointShader->setUniformValue(m_pointShader->nModel(), itModelMatrix.inverted().transposed());
		m_pointShader->setUniformValue(m_pointShader->color(), m_highlightColor);
		m_pointShader->setUniformValue(m_pointShader->MVP(), MVPMatrix);
		m_pointShader->setUniformValue(m_pointShader->ambientS(),
			m_cachedTheme->ambientLightStrength());
		m_pointShader->setUniformValue(m_pointShader->lightS(), m_cachedTheme->lightStrength() * 2.0f);
		m_pointShader->setUniformValue(m_pointShader->lightColor(),
			QUtils3D::vectorFromColor(m_cachedTheme->lightColor()));

		m_drawer->drawObject(m_pointShader, m_pointObj);
	}

	void QSelectionPointer3D::renderSelectionLabel(GLuint defaultFboHandle, bool useOrtho)
	{
		Q_UNUSED(defaultFboHandle)

			glViewport(m_mainViewPort.x(), m_mainViewPort.y(),
				m_mainViewPort.width(), m_mainViewPort.height());

		QCamera3D *camera = m_cachedScene->activeCamera();

		// Get view matrix
		QMatrix4x4 viewMatrix;
		QMatrix4x4 modelMatrixLabel;
		QMatrix4x4 projectionMatrix;
		GLfloat viewPortRatio = (GLfloat)m_mainViewPort.width() / (GLfloat)m_mainViewPort.height();
		if (m_cachedIsSlicingActivated) {
			GLfloat sliceUnitsScaled = sliceUnits / m_autoScaleAdjustment;
			viewMatrix.lookAt(QVector3D(0.0f, 0.0f, 1.0f), zeroVector, upVector);
			projectionMatrix.ortho(-sliceUnitsScaled * viewPortRatio, sliceUnitsScaled * viewPortRatio,
				-sliceUnitsScaled, sliceUnitsScaled,
				-1.0f, 4.0f);
		}
		else if (useOrtho) {
			viewMatrix = camera->d_ptr->viewMatrix();
			GLfloat orthoRatio = 2.0f;
			projectionMatrix.ortho(-viewPortRatio * orthoRatio, viewPortRatio * orthoRatio,
				-orthoRatio, orthoRatio,
				0.0f, 100.0f);
		}
		else {
			viewMatrix = camera->d_ptr->viewMatrix();
			projectionMatrix.perspective(45.0f, viewPortRatio, 0.1f, 100.0f);
		}

		QSize textureSize = m_labelItem.size();

		// Calculate scale factor to get uniform font size
		GLfloat scaledFontSize = 0.05f + m_drawer->font().pointSizeF() / 500.0f;
		GLfloat scaleFactor = scaledFontSize / (GLfloat)textureSize.height();

		// Position label
		QVector3D labelAlign(0.0f, 1.0f * scaledFontSize + 0.05f, 0.0f);
		modelMatrixLabel.translate(m_position + labelAlign);

		// Position the label towards the camera
		float camRotationsX = camera->xRotation();
		float camRotationsY = camera->yRotation();
		if (!m_cachedIsSlicingActivated) {
			modelMatrixLabel.rotate(-camRotationsX, 0.0f, 1.0f, 0.0f);
			modelMatrixLabel.rotate(-camRotationsY, 1.0f, 0.0f, 0.0f);
		}

		// Scale label based on text size
		modelMatrixLabel.scale(QVector3D((GLfloat)textureSize.width() * scaleFactor,
			scaledFontSize,
			0.0f));

		// Make label to be always on top
		glDisable(GL_DEPTH_TEST);

		// Make label transparent
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_labelShader->bind();

		QMatrix4x4 MVPMatrix;

		// Set shader bindings
		MVPMatrix = projectionMatrix * viewMatrix * modelMatrixLabel;
		m_labelShader->setUniformValue(m_labelShader->MVP(), MVPMatrix);

		// Draw the object
		m_drawer->drawObject(m_labelShader, m_labelObj, m_labelItem.textureId());

		// Release shader
		glUseProgram(0);

		// Disable transparency
		glDisable(GL_BLEND);

		// Depth test back to normal
		glEnable(GL_DEPTH_TEST);
	}

	void QSelectionPointer3D::setPosition(const QVector3D &position)
	{
		m_position = position;
	}

	void QSelectionPointer3D::updateSliceData(bool sliceActivated, GLfloat autoScaleAdjustment)
	{
		m_cachedIsSlicingActivated = sliceActivated;
		m_autoScaleAdjustment = autoScaleAdjustment;
	}

	void QSelectionPointer3D::setHighlightColor(const QVector4D &colorVector)
	{
		m_highlightColor = colorVector;
	}

	void QSelectionPointer3D::setRotation(const QQuaternion &rotation)
	{
		m_rotation = rotation;
	}

	void QSelectionPointer3D::setLabel(const QString &label, bool themeChange)
	{
		if (themeChange || m_label != label) {
			m_label = label;
			m_drawer->generateLabelItem(m_labelItem, m_label);
		}
	}

	void QSelectionPointer3D::setPointerObject(QObjectHelper3D *object)
	{
		m_pointObj = object;
	}

	void QSelectionPointer3D::setLabelObject(QObjectHelper3D *object)
	{
		m_labelObj = object;
	}

	void QSelectionPointer3D::handleDrawerChange()
	{
		m_cachedTheme = m_drawer->theme();
		setLabel(m_label, true);
	}

	void QSelectionPointer3D::updateBoundingRect(const QRect &rect)
	{
		m_mainViewPort = rect;
	}

	void QSelectionPointer3D::initShaders()
	{
		// The shader for printing the text label
		if (m_labelShader)
			delete m_labelShader;
		m_labelShader = new QShaderHelper3D(this, QStringLiteral(":/shaders/vertexLabel"),
			QStringLiteral(":/shaders/fragmentLabel"));
		m_labelShader->initialize();

		// The shader for the small point ball
		if (m_pointShader)
			delete m_pointShader;

		if (QUtils3D::isOpenGLES()) {
			m_pointShader = new QShaderHelper3D(this, QStringLiteral(":/shaders/vertex"),
				QStringLiteral(":/shaders/fragmentES2"));
		}
		else {
			m_pointShader = new QShaderHelper3D(this, QStringLiteral(":/shaders/vertex"),
				QStringLiteral(":/shaders/fragment"));
		}

		m_pointShader->initialize();
	}

}
