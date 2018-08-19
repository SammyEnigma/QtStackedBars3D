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

#include "QSeriesRenderCache3D.h"
#include "QAbstractSeries3DPrivate.h"
#include "QAbstractRenderer3D.h"
#include "QTextureHelper3D.h"
#include "QUtils3D.h"

namespace QtStackedBar3DVis
{
	const QString smoothString(QStringLiteral("Smooth"));

	QSeriesRenderCache3D::QSeriesRenderCache3D(QAbstractSeries3D *series, QAbstractRenderer3D *renderer)
		: m_series(series),
		m_object(0),
		m_mesh(QAbstractSeries3D::MeshCube),
		m_baseUniformTexture(0),
		m_baseGradientTexture(0),
		m_gradientImage(0),
		m_singleHighlightGradientTexture(0),
		m_multiHighlightGradientTexture(0),
		m_valid(false),
		m_visible(false),
		m_renderer(renderer),
		m_objectDirty(true),
		m_staticObjectUVDirty(false)
	{
	}

	QSeriesRenderCache3D::~QSeriesRenderCache3D()
	{
	}

	void QSeriesRenderCache3D::populate(bool newSeries)
	{
		QAbstractSeriesChangeBitField3D &changeTracker = m_series->d_ptr->m_changeTracker;

		if (newSeries || changeTracker.meshChanged || changeTracker.meshSmoothChanged
			|| changeTracker.userDefinedMeshChanged) {
			m_mesh = m_series->mesh();
			changeTracker.meshChanged = false;
			changeTracker.meshSmoothChanged = false;
			changeTracker.userDefinedMeshChanged = false;

			QString meshFileName;

			// Compose mesh filename
			if (m_mesh == QAbstractSeries3D::MeshUserDefined) {
				// Always use the supplied mesh directly
				meshFileName = m_series->userDefinedMesh();
			}
			else {
				switch (m_mesh) {
				case QAbstractSeries3D::MeshBar:
				case QAbstractSeries3D::MeshCube:
					meshFileName = QStringLiteral(":/defaultMeshes/bar");
					break;
				case QAbstractSeries3D::MeshPyramid:
					meshFileName = QStringLiteral(":/defaultMeshes/pyramid");
					break;
				case QAbstractSeries3D::MeshCone:
					meshFileName = QStringLiteral(":/defaultMeshes/cone");
					break;
				case QAbstractSeries3D::MeshCylinder:
					meshFileName = QStringLiteral(":/defaultMeshes/cylinder");
					break;
				case QAbstractSeries3D::MeshBevelBar:
				case QAbstractSeries3D::MeshBevelCube:
					meshFileName = QStringLiteral(":/defaultMeshes/bevelbar");
					break;
				case QAbstractSeries3D::MeshSphere:
					meshFileName = QStringLiteral(":/defaultMeshes/sphere");
					break;
				case QAbstractSeries3D::MeshMinimal:
					meshFileName = QStringLiteral(":/defaultMeshes/minimal");
					break;
				case QAbstractSeries3D::MeshArrow:
					meshFileName = QStringLiteral(":/defaultMeshes/arrow");
					break;
				case QAbstractSeries3D::MeshPoint:
					if (QUtils3D::isOpenGLES())
						qWarning("QAbstractSeries3D::MeshPoint is not fully supported on OpenGL ES2");
					break;
				default:
					// Default to cube
					meshFileName = QStringLiteral(":/defaultMeshes/bar");
					break;
				}

				if (m_series->isMeshSmooth() && m_mesh != QAbstractSeries3D::MeshPoint)
					meshFileName += smoothString;

				// Give renderer an opportunity to customize the mesh
				m_renderer->fixMeshFileName(meshFileName, m_mesh);
			}

			QObjectHelper3D::resetObjectHelper(m_renderer, m_object, meshFileName);
		}

		if (newSeries || changeTracker.meshRotationChanged) {
			m_meshRotation = m_series->meshRotation().normalized();
			if (m_series->type() == QAbstractSeries3D::SeriesTypeBar
				&& (m_meshRotation.x() || m_meshRotation.z())) {
				m_meshRotation = identityQuaternion;
			}
			changeTracker.meshRotationChanged = false;
		}

		if (newSeries || changeTracker.colorStyleChanged) {
			m_colorStyle = m_series->colorStyle();
			changeTracker.colorStyleChanged = false;
		}

		if (newSeries || changeTracker.baseColorChanged) {
			m_baseColor = QUtils3D::vectorFromColor(m_series->baseColor());
			if (m_series->type() == QAbstractSeries3D::SeriesTypeSurface)
				m_renderer->generateBaseColorTexture(m_series->baseColor(), &m_baseUniformTexture);
			changeTracker.baseColorChanged = false;
		}

		if (newSeries || changeTracker.baseGradientChanged) {
			QLinearGradient gradient = m_series->baseGradient();
			m_gradientImage = QUtils3D::getGradientImage(gradient);
			m_renderer->fixGradientAndGenerateTexture(&gradient, &m_baseGradientTexture);
			changeTracker.baseGradientChanged = false;
		}

		if (newSeries || changeTracker.singleHighlightColorChanged) {
			m_singleHighlightColor = QUtils3D::vectorFromColor(m_series->singleHighlightColor());
			changeTracker.singleHighlightColorChanged = false;
		}

		if (newSeries || changeTracker.singleHighlightGradientChanged) {
			QLinearGradient gradient = m_series->singleHighlightGradient();
			m_renderer->fixGradientAndGenerateTexture(&gradient, &m_singleHighlightGradientTexture);
			changeTracker.singleHighlightGradientChanged = false;
		}

		if (newSeries || changeTracker.multiHighlightColorChanged) {
			m_multiHighlightColor = QUtils3D::vectorFromColor(m_series->multiHighlightColor());
			changeTracker.multiHighlightColorChanged = false;
		}

		if (newSeries || changeTracker.multiHighlightGradientChanged) {
			QLinearGradient gradient = m_series->multiHighlightGradient();
			m_renderer->fixGradientAndGenerateTexture(&gradient, &m_multiHighlightGradientTexture);
			changeTracker.multiHighlightGradientChanged = false;
		}

		if (newSeries || changeTracker.nameChanged) {
			m_name = m_series->name();
			changeTracker.nameChanged = false;
		}

		if (newSeries || changeTracker.itemLabelChanged
			|| changeTracker.itemLabelVisibilityChanged) {
			changeTracker.itemLabelChanged = false;
			changeTracker.itemLabelVisibilityChanged = false;
			// series->itemLabel() call resolves the item label and emits the changed signal
			// if it is dirty, so we need to call it even if m_itemLabel is eventually set
			// to an empty string.
			m_itemLabel = m_series->itemLabel();
			if (!m_series->isItemLabelVisible())
				m_itemLabel = QString();
		}

		if (newSeries || changeTracker.visibilityChanged) {
			m_visible = m_series->isVisible();
			changeTracker.visibilityChanged = false;
		}
	}

	void QSeriesRenderCache3D::cleanup(QTextureHelper3D *texHelper)
	{
		QObjectHelper3D::releaseObjectHelper(m_renderer, m_object);
		if (QOpenGLContext::currentContext()) {
			texHelper->deleteTexture(&m_baseUniformTexture);
			texHelper->deleteTexture(&m_baseGradientTexture);
			texHelper->deleteTexture(&m_singleHighlightGradientTexture);
			texHelper->deleteTexture(&m_multiHighlightGradientTexture);
		}
	}

}
