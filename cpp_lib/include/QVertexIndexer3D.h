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

#ifndef VERTEXINDEXER_P_H
#define VERTEXINDEXER_P_H

#include "QVisualizationGlobals3D.h"

#include <QtCore/QVector>
#include <QtGui/QVector2D>

namespace QtStackedBar3DVis
{

	class QT_STACKEDBAR3D_EXPORT QVertexIndexer3D
	{
	public:
		struct PackedVertex {
			QVector3D position;
			QVector2D uv;
			QVector3D normal;
			bool operator<(const PackedVertex that) const {
				return memcmp((void*)this, (void*)&that, sizeof(PackedVertex)) > 0;
			}
		};

		static void indexVBO(const QVector<QVector3D> &in_vertices,
			const QVector<QVector2D> &in_uvs,
			const QVector<QVector3D> &in_normals,
			QVector<GLuint> &out_indices,
			QVector<QVector3D> &out_vertices,
			QVector<QVector2D> &out_uvs,
			QVector<QVector3D> &out_normals);

	private:
		static bool getSimilarVertexIndex_fast(const PackedVertex &packed,
			QMap<PackedVertex, GLuint> &VertexToOutIndex,
			GLuint &result);
	};

}

#endif
