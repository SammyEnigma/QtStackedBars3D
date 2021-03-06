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

#ifndef QTOUCH3DINPUTHANDLER_P_H
#define QTOUCH3DINPUTHANDLER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtDataVisualization API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#include "QInputHandler3DPrivate.h"
#include "QTouchInputHandler3D.h"

QT_FORWARD_DECLARE_CLASS(QTimer)

namespace QtStackedBar3DVis
{

	class QAbstractInputHandler3D;

	class QTouchInputHandler3DPrivate : public QInputHandler3DPrivate
	{
		Q_OBJECT

	public:
		QTouchInputHandler3DPrivate(QTouchInputHandler3D *q);
		~QTouchInputHandler3DPrivate();

		void handlePinchZoom(float distance, const QPoint &pos);
		void handleTapAndHold();
		void handleSelection(const QPointF &position);
		void handleRotation(const QPointF &position);

	private:
		QTouchInputHandler3D *q_ptr;
	public:
		QTimer *m_holdTimer;
		QAbstractInputHandler3DPrivate::InputState m_inputState;
		QPointF m_startHoldPos;
		QPointF m_touchHoldPos;
	};

}

#endif
