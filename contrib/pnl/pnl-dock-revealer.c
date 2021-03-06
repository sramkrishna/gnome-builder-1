/* pnl-dock-revealer.c
 *
 * Copyright (C) 2016 Christian Hergert <christian@hergert.me>
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

#include "pnl-animation.h"
#include "pnl-dock-revealer.h"
#include "pnl-util-private.h"

/**
 * SECTION:pnl-dock-revealer
 * @title: PnlDockRevealer
 * @short_description: A panel revealer
 *
 * This widget is a bit like #GtkRevealer with a couple of important
 * differences. First, it only supports a couple transition types
 * (the direction to slide reveal). Additionally, the size of the
 * child allocation will not change during the animation. This is not
 * as generally useful as an upstream GTK+ widget, but is extremely
 * important for the panel case to avoid things looking strange while
 * animating into and out of view.
 */

#define IS_HORIZONTAL(type) \
  (((type) == PNL_DOCK_REVEALER_TRANSITION_TYPE_SLIDE_RIGHT) || \
  ((type) == PNL_DOCK_REVEALER_TRANSITION_TYPE_SLIDE_LEFT))

#define IS_VERTICAL(type) \
  (((type) == PNL_DOCK_REVEALER_TRANSITION_TYPE_SLIDE_UP) || \
  ((type) == PNL_DOCK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN))

typedef struct
{
  PnlAnimation                  *animation;
  GtkAdjustment                 *adjustment;
  GdkWindow                     *window;
  gint                           position;
  gint                           position_tmp;
  guint                          transition_duration;
  PnlDockRevealerTransitionType  transition_type : 3;
  guint                          position_set : 1;
  guint                          reveal_child : 1;
  guint                          child_revealed : 1;
} PnlDockRevealerPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (PnlDockRevealer, pnl_dock_revealer, GTK_TYPE_BIN)

enum {
  PROP_0,
  PROP_CHILD_REVEALED,
  PROP_POSITION,
  PROP_POSITION_SET,
  PROP_REVEAL_CHILD,
  PROP_TRANSITION_DURATION,
  PROP_TRANSITION_TYPE,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

GtkWidget *
pnl_dock_revealer_new (void)
{
  return g_object_new (PNL_TYPE_DOCK_REVEALER, NULL);
}

guint
pnl_dock_revealer_get_transition_duration (PnlDockRevealer *self)
{
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);

  g_return_val_if_fail (PNL_IS_DOCK_REVEALER (self), 0);

  return priv->transition_duration;
}

void
pnl_dock_revealer_set_transition_duration (PnlDockRevealer *self,
                                           guint            transition_duration)
{
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);

  g_return_if_fail (PNL_IS_DOCK_REVEALER (self));

  if (priv->transition_duration != transition_duration)
    {
      priv->transition_duration = transition_duration;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_TRANSITION_DURATION]);
    }
}

PnlDockRevealerTransitionType
pnl_dock_revealer_get_transition_type (PnlDockRevealer *self)
{
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);

  g_return_val_if_fail (PNL_IS_DOCK_REVEALER (self), 0);

  return priv->transition_type;
}

void
pnl_dock_revealer_set_transition_type (PnlDockRevealer               *self,
                                       PnlDockRevealerTransitionType  transition_type)
{
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);

  g_return_if_fail (PNL_IS_DOCK_REVEALER (self));
  g_return_if_fail (transition_type >= PNL_DOCK_REVEALER_TRANSITION_TYPE_NONE);
  g_return_if_fail (transition_type <= PNL_DOCK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN);

  if (priv->transition_type != transition_type)
    {
      priv->transition_type = transition_type;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_TRANSITION_TYPE]);
    }
}

gboolean
pnl_dock_revealer_get_child_revealed (PnlDockRevealer *self)
{
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);

  g_return_val_if_fail (PNL_IS_DOCK_REVEALER (self), FALSE);

  return priv->child_revealed;
}

gboolean
pnl_dock_revealer_get_reveal_child (PnlDockRevealer *self)
{
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);

  g_return_val_if_fail (PNL_IS_DOCK_REVEALER (self), FALSE);

  return priv->reveal_child;
}

static void
pnl_dock_revealer_animation_done (gpointer user_data)
{
  g_autoptr(PnlDockRevealer) self = user_data;
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);

  g_assert (PNL_DOCK_REVEALER (self));

  if (priv->adjustment != NULL)
    {
      gboolean child_revealed;

      child_revealed = (gtk_adjustment_get_value (priv->adjustment) == 1.0);

      if (priv->child_revealed != child_revealed)
        {
          GtkWidget *child = gtk_bin_get_child (GTK_BIN (self));

          priv->child_revealed = child_revealed;
          gtk_widget_set_child_visible (GTK_WIDGET (child),
                                        gtk_adjustment_get_value (priv->adjustment) != 0.0);
          g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_CHILD_REVEALED]);
        }

      gtk_widget_queue_resize (GTK_WIDGET (self));
    }
}

static guint
size_to_duration (gint size)
{
  return MAX (150, size * 1.2);
}

static guint
pnl_dock_revealer_calculate_duration (PnlDockRevealer *self)
{
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);
  GtkWidget *child;
  GtkRequisition min_size;
  GtkRequisition nat_size;

  g_assert (PNL_IS_DOCK_REVEALER (self));

  child = gtk_bin_get_child (GTK_BIN (self));

  if (child == NULL)
    return 0;

  if (priv->transition_type == PNL_DOCK_REVEALER_TRANSITION_TYPE_NONE)
    return 0;

  if (priv->transition_duration != 0)
    return priv->transition_duration;

  gtk_widget_get_preferred_size (child, &min_size, &nat_size);

  if (IS_HORIZONTAL (priv->transition_type))
    {
      if (priv->position_set)
        {
          if (priv->position_set && priv->position > min_size.width)
            return size_to_duration (priv->position);
          return size_to_duration (min_size.width);
        }

      return size_to_duration (nat_size.width);
    }
  else
    {
      if (priv->position_set)
        {
          if (priv->position_set && priv->position > min_size.height)
            return size_to_duration (priv->position);
          return size_to_duration (min_size.height);
        }

      return size_to_duration (nat_size.height);
    }
}

void
pnl_dock_revealer_set_reveal_child (PnlDockRevealer *self,
                                    gboolean         reveal_child)
{
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);

  g_return_if_fail (PNL_IS_DOCK_REVEALER (self));

  reveal_child = !!reveal_child;

  if (reveal_child != priv->reveal_child)
    {
      PnlAnimation *animation;
      GtkWidget *child;

      priv->reveal_child = reveal_child;

      child = gtk_bin_get_child (GTK_BIN (self));

      if (child != NULL)
        {
          guint duration;

          if (priv->animation != NULL)
            {
              pnl_animation_stop (priv->animation);
              pnl_clear_weak_pointer (&priv->animation);
            }

          gtk_widget_set_child_visible (child, TRUE);

          duration = pnl_dock_revealer_calculate_duration (self);

          animation = pnl_object_animate_full (priv->adjustment,
                                               PNL_ANIMATION_EASE_IN_OUT_CUBIC,
                                               duration,
                                               gtk_widget_get_frame_clock (GTK_WIDGET (self)),
                                               pnl_dock_revealer_animation_done,
                                               g_object_ref (self),
                                               "value", reveal_child ? 1.0 : 0.0,
                                               NULL);

          pnl_set_weak_pointer (&priv->animation, animation);
        }

      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_REVEAL_CHILD]);
    }
}

gint
pnl_dock_revealer_get_position (PnlDockRevealer *self)
{
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);

  g_return_val_if_fail (PNL_IS_DOCK_REVEALER (self), 0);

  return priv->position;
}

void
pnl_dock_revealer_set_position (PnlDockRevealer *self,
                                gint             position)
{
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);

  g_return_if_fail (PNL_IS_DOCK_REVEALER (self));
  g_return_if_fail (position >= 0);

  if (priv->position != position)
    {
      priv->position = position;

      if (!priv->position_set)
        {
          priv->position_set = TRUE;
          g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_POSITION_SET]);
        }

      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_POSITION]);
      gtk_widget_queue_resize (GTK_WIDGET (self));
    }
}

gboolean
pnl_dock_revealer_get_position_set (PnlDockRevealer *self)
{
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);

  g_return_val_if_fail (PNL_IS_DOCK_REVEALER (self), FALSE);

  return priv->position_set;
}

void
pnl_dock_revealer_set_position_set (PnlDockRevealer *self,
                                    gboolean         position_set)
{
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);

  g_return_if_fail (PNL_IS_DOCK_REVEALER (self));

  position_set = !!position_set;

  if (priv->position_set != position_set)
    {
      priv->position_set = position_set;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_POSITION_SET]);
      gtk_widget_queue_resize (GTK_WIDGET (self));
    }
}

static void
pnl_dock_revealer_get_child_preferred_width (PnlDockRevealer *self,
                                             gint            *min_width,
                                             gint            *nat_width)
{
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);
  GtkWidget *child;

  g_assert (PNL_IS_DOCK_REVEALER (self));
  g_assert (min_width != NULL);
  g_assert (nat_width != NULL);

  *min_width = 0;
  *nat_width = 0;

  if (NULL == (child = gtk_bin_get_child (GTK_BIN (self))))
    return;

  if (!gtk_widget_get_child_visible (child) || !gtk_widget_get_visible (child))
    return;

  gtk_widget_get_preferred_width (child, min_width, nat_width);

  if (IS_HORIZONTAL (priv->transition_type) && priv->position_set)
    {
      if (priv->position > *min_width)
        *nat_width = priv->position;
      else
        *nat_width = *min_width;
    }
}

static void
pnl_dock_revealer_get_preferred_width (GtkWidget *widget,
                                       gint      *min_width,
                                       gint      *nat_width)
{
  PnlDockRevealer *self = (PnlDockRevealer *)widget;
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);

  g_assert (PNL_IS_DOCK_REVEALER (self));
  g_assert (min_width != NULL);
  g_assert (nat_width != NULL);

  pnl_dock_revealer_get_child_preferred_width (self, min_width, nat_width);

  if (IS_HORIZONTAL (priv->transition_type) && priv->animation != NULL)
    {
      /*
       * We allow going smaller than the minimum size during animations
       * and rely on clipping to hide the child.
       */
      *min_width = 0;

      /*
       * Our natural width is adjusted for the in-progress animation.
       */
      *nat_width *= gtk_adjustment_get_value (priv->adjustment);
    }
}

static void
pnl_dock_revealer_get_child_preferred_height (PnlDockRevealer *self,
                                              gint            *min_height,
                                              gint            *nat_height)
{
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);
  GtkWidget *child;

  g_assert (PNL_IS_DOCK_REVEALER (self));
  g_assert (min_height != NULL);
  g_assert (nat_height != NULL);

  *min_height = 0;
  *nat_height = 0;

  if (NULL == (child = gtk_bin_get_child (GTK_BIN (self))))
    return;

  if (!gtk_widget_get_child_visible (child) || !gtk_widget_get_visible (child))
    return;

  gtk_widget_get_preferred_height (child, min_height, nat_height);

  if (IS_VERTICAL (priv->transition_type) && priv->position_set)
    {
      if (priv->position > *min_height)
        *nat_height = priv->position;
      else
        *nat_height = *min_height;
    }
}

static void
pnl_dock_revealer_get_preferred_height (GtkWidget *widget,
                                        gint      *min_height,
                                        gint      *nat_height)
{
  PnlDockRevealer *self = (PnlDockRevealer *)widget;
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);

  g_assert (PNL_IS_DOCK_REVEALER (self));
  g_assert (min_height != NULL);
  g_assert (nat_height != NULL);

  pnl_dock_revealer_get_child_preferred_height (self, min_height, nat_height);

  if (IS_VERTICAL (priv->transition_type) && priv->animation != NULL)
    {
      /*
       * We allow going smaller than the minimum size during animations
       * and rely on clipping to hide the child.
       */
      *min_height = 0;

      /*
       * Our natural height is adjusted for the in-progress animation.
       */
      *nat_height *= gtk_adjustment_get_value (priv->adjustment);
    }
}

static void
pnl_dock_revealer_size_allocate (GtkWidget     *widget,
                                 GtkAllocation *allocation)
{
  PnlDockRevealer *self = (PnlDockRevealer *)widget;
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);
  GtkAllocation child_allocation;
  GtkRequisition min_req;
  GtkRequisition nat_req;
  GtkWidget *child;

  g_assert (PNL_IS_DOCK_REVEALER (self));

  gtk_widget_set_allocation (widget, allocation);

  if (gtk_widget_get_realized (GTK_WIDGET (self)))
    gdk_window_move_resize (priv->window,
                            allocation->x,
                            allocation->y,
                            allocation->width,
                            allocation->height);

  if (NULL == (child = gtk_bin_get_child (GTK_BIN (self))))
    return;

  if (!gtk_widget_get_child_visible (child))
    return;

  child_allocation.x = 0;
  child_allocation.y = 0;
  child_allocation.width = allocation->width;
  child_allocation.height = allocation->height;

  if (IS_HORIZONTAL (priv->transition_type))
    {
      pnl_dock_revealer_get_child_preferred_width (self, &min_req.width, &nat_req.width);
      child_allocation.width = nat_req.width;

      if (priv->transition_type == PNL_DOCK_REVEALER_TRANSITION_TYPE_SLIDE_RIGHT)
        child_allocation.x = allocation->width - child_allocation.width;
    }
  else if (IS_VERTICAL (priv->transition_type))
    {
      pnl_dock_revealer_get_child_preferred_height (self, &min_req.height, &nat_req.height);
      child_allocation.height = nat_req.height;

      if (priv->transition_type == PNL_DOCK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN)
        child_allocation.y = allocation->height - child_allocation.height;
    }

  gtk_widget_size_allocate (child, &child_allocation);
}

static void
pnl_dock_revealer_add (GtkContainer *container,
                       GtkWidget    *widget)
{
  PnlDockRevealer *self = (PnlDockRevealer *)container;
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);

  g_assert (PNL_IS_DOCK_REVEALER (self));
  g_assert (GTK_IS_WIDGET (widget));

  GTK_CONTAINER_CLASS (pnl_dock_revealer_parent_class)->add (container, widget);

  gtk_widget_set_child_visible (widget, priv->reveal_child);
}

static void
pnl_dock_revealer_realize (GtkWidget *widget)
{
  PnlDockRevealer *self = (PnlDockRevealer *)widget;
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);
  GdkWindowAttr attributes = { 0 };
  GdkWindow *parent;
  GtkAllocation alloc;
  gint attributes_mask = 0;

  g_assert (PNL_IS_DOCK_REVEALER (widget));

  gtk_widget_get_allocation (GTK_WIDGET (self), &alloc);

  gtk_widget_set_realized (GTK_WIDGET (self), TRUE);

  parent = gtk_widget_get_parent_window (GTK_WIDGET (self));

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual (GTK_WIDGET (self));
  attributes.x = alloc.x;
  attributes.y = alloc.y;
  attributes.width = alloc.width;
  attributes.height = alloc.height;
  attributes.event_mask = 0;

  attributes_mask = (GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL);

  priv->window = gdk_window_new (parent, &attributes, attributes_mask);
  gtk_widget_set_window (GTK_WIDGET (self), priv->window);
  gtk_widget_register_window (GTK_WIDGET (self), priv->window);
}

static void
pnl_dock_revealer_destroy (GtkWidget *widget)
{
  PnlDockRevealer *self = (PnlDockRevealer *)widget;
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);

  g_clear_object (&priv->adjustment);
  pnl_clear_weak_pointer (&priv->animation);

  GTK_WIDGET_CLASS (pnl_dock_revealer_parent_class)->destroy (widget);
}

static void
pnl_dock_revealer_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  PnlDockRevealer *self = PNL_DOCK_REVEALER (object);

  switch (prop_id)
    {
    case PROP_CHILD_REVEALED:
      g_value_set_boolean (value, pnl_dock_revealer_get_child_revealed (self));
      break;

    case PROP_POSITION:
      g_value_set_int (value, pnl_dock_revealer_get_position (self));
      break;

    case PROP_POSITION_SET:
      g_value_set_boolean (value, pnl_dock_revealer_get_position_set (self));
      break;

    case PROP_REVEAL_CHILD:
      g_value_set_boolean (value, pnl_dock_revealer_get_reveal_child (self));
      break;

    case PROP_TRANSITION_DURATION:
      g_value_set_uint (value, pnl_dock_revealer_get_transition_duration (self));
      break;

    case PROP_TRANSITION_TYPE:
      g_value_set_enum (value, pnl_dock_revealer_get_transition_type (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_dock_revealer_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  PnlDockRevealer *self = PNL_DOCK_REVEALER (object);

  switch (prop_id)
    {
    case PROP_REVEAL_CHILD:
      pnl_dock_revealer_set_reveal_child (self, g_value_get_boolean (value));
      break;

    case PROP_POSITION:
      pnl_dock_revealer_set_position (self, g_value_get_int (value));
      break;

    case PROP_POSITION_SET:
      pnl_dock_revealer_set_position_set (self, g_value_get_boolean (value));
      break;

    case PROP_TRANSITION_DURATION:
      pnl_dock_revealer_set_transition_duration (self, g_value_get_uint (value));
      break;

    case PROP_TRANSITION_TYPE:
      pnl_dock_revealer_set_transition_type (self, g_value_get_enum (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_dock_revealer_class_init (PnlDockRevealerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->get_property = pnl_dock_revealer_get_property;
  object_class->set_property = pnl_dock_revealer_set_property;

  widget_class->destroy = pnl_dock_revealer_destroy;
  widget_class->get_preferred_width = pnl_dock_revealer_get_preferred_width;
  widget_class->get_preferred_height = pnl_dock_revealer_get_preferred_height;
  widget_class->realize = pnl_dock_revealer_realize;
  widget_class->size_allocate = pnl_dock_revealer_size_allocate;

  container_class->add = pnl_dock_revealer_add;

  properties [PROP_CHILD_REVEALED] =
    g_param_spec_boolean ("child-revealed",
                          "Child Revealed",
                          "If the child is fully revealed",
                          TRUE,
                          (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  properties [PROP_POSITION] =
    g_param_spec_int ("position",
                      "Position",
                      "Position",
                      0,
                      G_MAXINT,
                      0,
                      (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_POSITION_SET] =
    g_param_spec_boolean ("position-set",
                          "Position Set",
                          "If the position has been set",
                          FALSE,
                          (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_REVEAL_CHILD] =
    g_param_spec_boolean ("reveal-child",
                          "Reveal Child",
                          "If the child should be revealed",
                          FALSE,
                          (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_TRANSITION_DURATION] =
    g_param_spec_uint ("transition-duration",
                       "Transition Duration",
                       "Length of duration in milliseconds",
                       0,
                       G_MAXUINT,
                       0,
                       (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_TRANSITION_TYPE] =
    g_param_spec_enum ("transition-type",
                       "Transition Type",
                       "Transition Type",
                       PNL_TYPE_DOCK_REVEALER_TRANSITION_TYPE,
                       PNL_DOCK_REVEALER_TRANSITION_TYPE_NONE,
                       (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
pnl_dock_revealer_init (PnlDockRevealer *self)
{
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);

  gtk_widget_set_has_window (GTK_WIDGET (self), TRUE);

  priv->reveal_child = FALSE;
  priv->child_revealed = FALSE;

  priv->transition_duration = 0;

  priv->adjustment = g_object_new (GTK_TYPE_ADJUSTMENT,
                                   "lower", 0.0,
                                   "upper", 1.0,
                                   "value", 0.0,
                                   NULL);

  g_signal_connect_object (priv->adjustment,
                           "value-changed",
                           G_CALLBACK (gtk_widget_queue_resize),
                           self,
                           G_CONNECT_SWAPPED);
}

GType
pnl_dock_revealer_transition_type_get_type (void)
{
  static GType type_id;

  if (g_once_init_enter (&type_id))
    {
      GType _type_id;
      static const GEnumValue values[] = {
        { PNL_DOCK_REVEALER_TRANSITION_TYPE_NONE,
          "PNL_DOCK_REVEALER_TRANSITION_TYPE_NONE",
          "none" },
        { PNL_DOCK_REVEALER_TRANSITION_TYPE_SLIDE_RIGHT,
          "PNL_DOCK_REVEALER_TRANSITION_TYPE_SLIDE_RIGHT",
          "slide-right" },
        { PNL_DOCK_REVEALER_TRANSITION_TYPE_SLIDE_LEFT,
          "PNL_DOCK_REVEALER_TRANSITION_TYPE_SLIDE_LEFT",
          "slide-left" },
        { PNL_DOCK_REVEALER_TRANSITION_TYPE_SLIDE_UP,
          "PNL_DOCK_REVEALER_TRANSITION_TYPE_SLIDE_UP",
          "slide-up" },
        { PNL_DOCK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN,
          "PNL_DOCK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN",
          "slide-down" },
        { 0 }
      };

      _type_id = g_enum_register_static ("PnlDockRevealerTransitionType", values);

      g_once_init_leave (&type_id, _type_id);
    }

  return type_id;
}

static void
pnl_dock_revealer_animate_to_position_done (gpointer user_data)
{
  g_autoptr(PnlDockRevealer) self = user_data;
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);

  g_assert (PNL_DOCK_REVEALER (self));

  if (priv->adjustment != NULL)
    {
      gboolean child_revealed;

      child_revealed = (priv->position_tmp > 0);
      if (priv->child_revealed != child_revealed)
        {
          GtkWidget *child = gtk_bin_get_child (GTK_BIN (self));

          priv->child_revealed = child_revealed;
          gtk_widget_set_child_visible (GTK_WIDGET (child), child_revealed);
        }

      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_CHILD_REVEALED]);

      gtk_adjustment_set_value (priv->adjustment, child_revealed ? 1.0 : 0.0);
      priv->position = priv->position_tmp;
      gtk_widget_queue_resize (GTK_WIDGET (self));
    }
}

void
pnl_dock_revealer_animate_to_position (PnlDockRevealer *self,
                                       gint             position,
                                       guint            transition_duration)
{
  PnlDockRevealerPrivate *priv = pnl_dock_revealer_get_instance_private (self);
  gdouble current_position;
  gdouble value;

  g_return_if_fail (PNL_IS_DOCK_REVEALER (self));

  if (transition_duration == 0)
    transition_duration = pnl_dock_revealer_calculate_duration (self);

  current_position = priv->position;
  if (current_position != position)
    {
      PnlAnimation *animation;
      GtkWidget *child;

      priv->reveal_child = (position > 0);
      priv->position_tmp = position;
      if (!priv->position_set)
        {
          priv->position_set = TRUE;
          g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_POSITION_SET]);
        }

      if (current_position < position)
        {
          value = 1.0;
          if (current_position > 0)
            {
              priv->position = position;
              gtk_adjustment_set_value (priv->adjustment, current_position / position);
            }
        }
      else
        value = position / current_position;

      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_POSITION]);

      child = gtk_bin_get_child (GTK_BIN (self));
      if (child != NULL)
        {
          if (priv->animation != NULL)
            {
              pnl_animation_stop (priv->animation);
              pnl_clear_weak_pointer (&priv->animation);
            }

          gtk_widget_set_child_visible (child, TRUE);
          animation = pnl_object_animate_full (priv->adjustment,
                                               PNL_ANIMATION_EASE_IN_OUT_CUBIC,
                                               transition_duration,
                                               gtk_widget_get_frame_clock (GTK_WIDGET (self)),
                                               pnl_dock_revealer_animate_to_position_done,
                                               g_object_ref (self),
                                               "value", value,
                                               NULL);

          pnl_set_weak_pointer (&priv->animation, animation);
        }

      if ((priv->reveal_child && position == 0) || (!priv->reveal_child && position != 0))
        g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_REVEAL_CHILD]);
    }
}
