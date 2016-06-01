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

#ifndef __AS_RECOMMENDATION_FILE_H
#define __AS_RECOMMENDATION_FILE_H

#include <glib-object.h>
#include <libxml/tree.h>

G_BEGIN_DECLS

#define AS_TYPE_RECOMMENDATION_FILE (as_recommendation_file_get_type ())
G_DECLARE_FINAL_TYPE (AsRecommendationFile, as_recommendation_file,
					  AS, RECOMMENDATION_FILE, GObject)

struct _AsRecommendationFile
{
	GObjectClass		parent_class;
};

AsRecommendationFile		*as_recommendation_file_new (void);

void						as_recommendation_file_initialize (AsRecommendationFile *rec_file,
												   const gchar *origin);

GPtrArray					*as_recommendation_file_parse_file (AsRecommendationFile *rec_file,
													const gchar *data,
													GError **error);

xmlDoc						*as_recommendation_file_parse_document (AsRecommendationFile *rec_file,
														const gchar *data,
														GError **error);

void						as_recommendation_file_parse_recommendations_node (AsRecommendationFile *rec_file,
												   GPtrArray *recs, xmlNode* node, GError **error);

gchar**						as_recommendation_file_get_children_as_strv (AsRecommendationFile *rec_file,
																		 xmlNode* node,
																		 const gchar* element_name);
G_END_DECLS

#endif
