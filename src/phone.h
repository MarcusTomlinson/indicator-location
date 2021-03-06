/*
 * Copyright 2013-2016 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *   Charles Kerr <charles.kerr@canonical.com>
 */

#pragma once

#include <memory>
#include <vector>

#include <glib.h>
#include <gio/gio.h>

#include "controller.h"

class Phone
{
public:
    Phone(const std::shared_ptr<Controller>& controller, const std::shared_ptr<GSimpleActionGroup>& action_group);
    virtual ~Phone();
    std::shared_ptr<GMenu> get_menu()
    {
        return menu;
    }

protected:
    std::shared_ptr<Controller> controller;
    std::vector<core::ScopedConnection> controller_connections;

private:
    std::shared_ptr<GMenu> menu;
    std::shared_ptr<GMenu> submenu;
    std::shared_ptr<GSimpleActionGroup> action_group;

private:
    void create_menu();
    void rebuild_submenu();

private:
    bool should_be_visible() const;
    bool location_service_active() const;
    GVariant* action_state_for_root() const;
    GSimpleAction* create_root_action();
    void update_header();
    void update_actions_enabled();

private:
    GVariant* action_state_for_location_detection();
    GSimpleAction* create_detection_enabled_action();
    void update_detection_enabled_action();
    static void on_detection_location_activated(GSimpleAction*, GVariant*, gpointer);

private:
    GVariant* action_state_for_gps_detection();
    GSimpleAction* create_gps_enabled_action();
    void update_gps_enabled_action();
    static void on_detection_gps_activated(GSimpleAction*, GVariant*, gpointer);

private:
    GSimpleAction* create_settings_action();

private:
    GSimpleAction* create_licence_action();
};
