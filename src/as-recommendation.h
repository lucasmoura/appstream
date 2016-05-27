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

#ifndef __AS_RECOMMENDATION_H
#define __AS_RECOMMENDATION_H

#include <glib-object.h>

G_BEGIN_DECLS

#define AS_TYPE_RECOMMENDATION (as_recommendation_get_type ())
G_DECLARE_FINAL_TYPE (AsRecommendation, as_recommendation, AS, RECOMMENDATION, GObject)

struct _AsRecommendation
{
	GObjectClass		parent_class;
};

AsRecommendation		*as_recommendation_new (void);

const gchar*		as_recommendation_get_recommended_package (AsRecommendation *rec);
void				as_recommendation_set_recommended_package (AsRecommendation *rec,
									 const gchar *value);

gchar				**as_recommendation_get_because (AsRecommendation *rec);
void				as_recommendation_set_because (AsRecommendation *rec, gchar **value);

G_END_DECLS

#endif
