/* ide-glib.c
 *
 * Copyright (C) 2016 Christian Hergert <chergert@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define G_LOG_DOMAIN "ide-glib"

#include "config.h"

#include "ide-glib.h"

typedef struct
{
  int type;
  GTask *task;
  union {
    gboolean v_bool;
    gint v_int;
    GError *v_error;
    struct {
      gpointer pointer;
      GDestroyNotify destroy;
    } v_ptr;
  } u;
} TaskState;

static gboolean
do_return (gpointer user_data)
{
  TaskState *state = user_data;

  switch (state->type)
    {
    case G_TYPE_INT:
      g_task_return_int (state->task, state->u.v_int);
      break;

    case G_TYPE_BOOLEAN:
      g_task_return_boolean (state->task, state->u.v_bool);
      break;

    case G_TYPE_POINTER:
      g_task_return_pointer (state->task, state->u.v_ptr.pointer, state->u.v_ptr.destroy);
      state->u.v_ptr.pointer = NULL;
      state->u.v_ptr.destroy = NULL;
      break;

    default:
      if (state->type == G_TYPE_ERROR)
        {
          g_task_return_error (state->task, state->u.v_error);
          state->u.v_error = NULL;
          break;
        }

      g_assert_not_reached ();
    }

  g_clear_object (&state->task);
  g_slice_free (TaskState, state);

  return G_SOURCE_REMOVE;
}

static void
task_state_attach (TaskState *state)
{
  GMainContext *main_context;
  GSource *source;

  g_assert (state != NULL);
  g_assert (G_IS_TASK (state->task));

  main_context = g_task_get_context (state->task);

  source = g_timeout_source_new (0);
  g_source_set_callback (source, do_return, state, NULL);
  g_source_set_name (source, "[ide] ide_g_task_return_from_main");
  g_source_attach (source, main_context);
  g_source_unref (source);
}

/**
 * ide_g_task_return_boolean_from_main:
 *
 * This is just like g_task_return_boolean() except that it enforces
 * that the current stack return to the main context before dispatching
 * the callback.
 */
void
ide_g_task_return_boolean_from_main (GTask    *task,
                                     gboolean  value)
{
  TaskState *state;

  g_return_if_fail (G_IS_TASK (task));

  state = g_slice_new0 (TaskState);
  state->type = G_TYPE_BOOLEAN;
  state->task = g_object_ref (task);
  state->u.v_bool = !!value;

  task_state_attach (state);
}

void
ide_g_task_return_int_from_main (GTask *task,
                                 gint   value)
{
  TaskState *state;

  g_return_if_fail (G_IS_TASK (task));

  state = g_slice_new0 (TaskState);
  state->type = G_TYPE_INT;
  state->task = g_object_ref (task);
  state->u.v_int = value;

  task_state_attach (state);
}

void
ide_g_task_return_pointer_from_main (GTask          *task,
                                     gpointer        value,
                                     GDestroyNotify  notify)
{
  TaskState *state;

  g_return_if_fail (G_IS_TASK (task));

  state = g_slice_new0 (TaskState);
  state->type = G_TYPE_POINTER;
  state->task = g_object_ref (task);
  state->u.v_ptr.pointer = value;
  state->u.v_ptr.destroy = notify;

  task_state_attach (state);
}

/**
 * ide_g_task_return_error_from_main:
 * @task: A #GTask
 * @error: (transfer full): A #GError.
 *
 * Like g_task_return_error() but ensures we return to the main loop before
 * dispatching the result.
 */
void
ide_g_task_return_error_from_main (GTask  *task,
                                   GError *error)
{
  TaskState *state;

  g_return_if_fail (G_IS_TASK (task));

  state = g_slice_new0 (TaskState);
  state->type = G_TYPE_ERROR;
  state->task = g_object_ref (task);
  state->u.v_error = error;

  task_state_attach (state);
}

const gchar *
ide_gettext (const gchar *message)
{
  if (message != NULL)
    return g_dgettext (GETTEXT_PACKAGE, message);
  return NULL;
}

gchar *
ide_g_time_span_to_label (GTimeSpan span)
{
  gint64 hours;
  gint64 minutes;
  gint64 seconds;

  span = ABS (span);

  hours = span / G_TIME_SPAN_HOUR;
  minutes = (span % G_TIME_SPAN_HOUR) / G_TIME_SPAN_MINUTE;
  seconds = (span % G_TIME_SPAN_MINUTE) / G_TIME_SPAN_SECOND;

  g_assert (minutes < 60);
  g_assert (seconds < 60);

  if (hours == 0)
    return g_strdup_printf ("%02"G_GINT64_FORMAT":%02"G_GINT64_FORMAT,
                            minutes, seconds);
  else
    return g_strdup_printf ("%02"G_GINT64_FORMAT":%02"G_GINT64_FORMAT":%02"G_GINT64_FORMAT,
                            hours, minutes, seconds);
}

gboolean
ide_g_time_span_to_label_mapping (GBinding     *binding,
                                  const GValue *from_value,
                                  GValue       *to_value,
                                  gpointer      user_data)
{
  GTimeSpan span;

  g_assert (G_IS_BINDING (binding));
  g_assert (from_value != NULL);
  g_assert (G_VALUE_HOLDS (from_value, G_TYPE_INT64));
  g_assert (to_value != NULL);
  g_assert (G_VALUE_HOLDS (to_value, G_TYPE_STRING));

  span = g_value_get_int64 (from_value);
  g_value_take_string (to_value, ide_g_time_span_to_label (span));

  return TRUE;
}
