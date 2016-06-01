/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2016 Lucas Moura <lucas.moura128@gmail.com>
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the license, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#if !defined (__APPSTREAM_H) && !defined (AS_COMPILATION)
#error "Only <appstream.h> can be included directly."
#endif

#ifndef __AS_RECOMMENDATION_METADATA_H
#define __AS_RECOMMENDATION_METADATA_H

#include <glib.h>
#include <gio/gio.h>
#include "as-recommendation.h"

G_BEGIN_DECLS

#define AS_TYPE_RECOMMENDATION_METADATA (as_recommendation_metadata_get_type ())
G_DECLARE_FINAL_TYPE (AsRecommendationMetadata, as_recommendation_metadata, AS, RECOMMENDATION_METADATA, GObject)

struct _AsRecommendationMetadata
{
	GObjectClass		parent_class;
};


AsRecommendationMetadata		*as_recommendation_metadata_new (void);

GPtrArray		*as_recommendation_metadata_get_components (AsRecommendationMetadata *rec_metad);

void			as_recommendationmetadata_add_component (AsRecommendationMetadata *rec_metad,
							AsRecommendation *rec);

const gchar		*as_recommendation_metadata_get_origin (AsRecommendationMetadata *rec_metad);
void			as_recommendation_metadata_set_origin (AsRecommendationMetadata *rec_metad,
							const gchar *origin);

void			as_recommendation_metadata_parse_file (AsRecommendationMetadata *rec_metad,
													   GFile *file,
													   GError **error);

void			as_recommendation_metadata_parse_rec (AsRecommendationMetadata *rec_metad,
													  const gchar *data,
													  GError **error);

G_END_DECLS

#endif
