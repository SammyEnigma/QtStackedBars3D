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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtDataVisualization API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef Q3DCAMERA_P_H
#define Q3DCAMERA_P_H

#include "QVisualizationGlobals3D.h"
#include "QCamera3D.h"
#include <QtGui/QMatrix4x4>

namespace QtStackedBar3DVis
{

	class QCamera3D;

	class QCamera3DPrivate : public QObject
	{
		Q_OBJECT
	public:
		QCamera3DPrivate(QCamera3D *q);
		~QCamera3DPrivate();

		void sync(QCamera3D &other);

		void setXRotation(float rotation);
		void setYRotation(float rotation);
		void setMinXRotation(float rotation);
		float minXRotation() const;
		void setMinYRotation(float rotation);
		float minYRotation() const;
		void setMaxXRotation(float rotation);
		float maxXRotation() const;
		void setMaxYRotation(float rotation);
		float maxYRotation() const;

		void updateViewMatrix(float zoomAdjustment);

		QMatrix4x4 viewMatrix() const;
		void setViewMatrix(const QMatrix4x4 &viewMatrix);

		bool isViewMatrixAutoUpdateEnabled() const;
		void setViewMatrixAutoUpdateEnabled(bool isEnabled);

		void setBaseOrientation(const QVector3D &defaultPosition,
			const QVector3D &defaultTarget,
			const QVector3D &defaultUp);

		QVector3D calculatePositionRelativeToCamera(const QVector3D &relativePosition,
			float fixedRotation,
			float distanceModifier) const;

	Q_SIGNALS:
		void minXRotationChanged(float rotation);
		void minYRotationChanged(float rotation);
		void maxXRotationChanged(float rotation);
		void maxYRotationChanged(float rotation);
		void viewMatrixChanged(QMatrix4x4 viewMatrix);
		void viewMatrixAutoUpdateChanged(bool enabled);

	public:
		QCamera3D *q_ptr;

		QVector3D m_actualTarget;
		QVector3D m_up;

		QMatrix4x4 m_viewMatrix;
		bool m_isViewMatrixUpdateActive;

		float m_xRotation;
		float m_yRotation;
		float m_minXRotation;
		float m_minYRotation;
		float m_maxXRotation;
		float m_maxYRotation;
		float m_zoomLevel;
		float m_minZoomLevel;
		float m_maxZoomLevel;
		bool m_wrapXRotation;
		bool m_wrapYRotation;
		QCamera3D::CameraPreset m_activePreset;
		QVector3D m_requestedTarget;

		friend class QBarsRenderer3D;
		friend class QStackedBarsRenderer3D;
		friend class QSurfaceRenderer3D;
		friend class QScatterRenderer3D;
		friend class QSelectionPointer3D;
		friend class QInputHandler3D;
		friend class QTouchInputHandler3D;
	};
}

#endif
