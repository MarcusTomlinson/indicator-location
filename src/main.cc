/*
 * Copyright 2013 Canonical Ltd.
 *
 * Authors:
 *   Charles Kerr <charles.kerr@canonical.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <locale.h>
#include <glib/gi18n.h>
#include <glib.h>

#include "controller-ualc.h"
#include "service.h"

static void
on_name_lost (Service * service G_GNUC_UNUSED, gpointer loop)
{
  g_main_loop_quit (static_cast<GMainLoop*>(loop));
}

int
main (int argc G_GNUC_UNUSED, char ** argv G_GNUC_UNUSED)
{
  GMainLoop * loop;

  /* boilerplate i18n */
  setlocale (LC_ALL, "");
  bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
  textdomain (GETTEXT_PACKAGE);
 
  /* set up the service */
  loop = g_main_loop_new (nullptr, false);
  std::shared_ptr<Controller> controller (new UbuntuAppLocController ());
  Service service (controller);
  service.set_name_lost_callback (on_name_lost, loop);
  g_main_loop_run (loop);

  /* cleanup */
  g_main_loop_unref (loop);
  return 0;
}
