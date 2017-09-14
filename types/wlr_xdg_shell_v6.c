#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-server.h>
#include <wlr/types/wlr_xdg_shell_v6.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/util/log.h>
#include "xdg-shell-unstable-v6-protocol.h"

static const char *wlr_desktop_xdg_toplevel_role = "xdg_toplevel";

static void resource_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	// TODO: we probably need to do more than this
	wl_resource_destroy(resource);
}

static void xdg_toplevel_set_parent(struct wl_client *client,
		struct wl_resource *resource, struct wl_resource *parent_resource) {
	wlr_log(L_DEBUG, "TODO: toplevel set parent");
}

static void xdg_toplevel_set_title(struct wl_client *client,
		struct wl_resource *resource, const char *title) {
	struct wlr_xdg_surface_v6 *surface = wl_resource_get_user_data(resource);
	char *tmp;

	tmp = strdup(title);
	if (tmp == NULL) {
		return;
	}

	free(surface->title);
	surface->title = tmp;
}

static void xdg_toplevel_set_app_id(struct wl_client *client,
		struct wl_resource *resource, const char *app_id) {
	struct wlr_xdg_surface_v6 *surface = wl_resource_get_user_data(resource);
	char *tmp;

	tmp = strdup(app_id);
	if (tmp == NULL) {
		return;
	}

	free(surface->app_id);
	surface->app_id = tmp;
}

static void xdg_toplevel_show_window_menu(struct wl_client *client,
		struct wl_resource *resource, struct wl_resource *seat, uint32_t serial,
		int32_t x, int32_t y) {
	wlr_log(L_DEBUG, "TODO: toplevel show window menu");
}

static void xdg_toplevel_move(struct wl_client *client,
		struct wl_resource *resource, struct wl_resource *seat_resource,
		uint32_t serial) {
	wlr_log(L_DEBUG, "TODO: toplevel move");
}

static void xdg_toplevel_resize(struct wl_client *client,
		struct wl_resource *resource, struct wl_resource *seat_resource,
		uint32_t serial, uint32_t edges) {
	wlr_log(L_DEBUG, "TODO: toplevel resize");
}

static void xdg_toplevel_set_max_size(struct wl_client *client,
		struct wl_resource *resource, int32_t width, int32_t height) {
	struct wlr_xdg_surface_v6 *surface = wl_resource_get_user_data(resource);
	surface->toplevel_state->next.max_width = width;
	surface->toplevel_state->next.max_height = height;
}

static void xdg_toplevel_set_min_size(struct wl_client *client,
		struct wl_resource *resource, int32_t width, int32_t height) {
	struct wlr_xdg_surface_v6 *surface = wl_resource_get_user_data(resource);
	surface->toplevel_state->next.min_width = width;
	surface->toplevel_state->next.min_height = height;
}

static void xdg_toplevel_set_maximized(struct wl_client *client,
		struct wl_resource *resource) {
	struct wlr_xdg_surface_v6 *surface = wl_resource_get_user_data(resource);
	surface->toplevel_state->next.maximized = true;
}

static void xdg_toplevel_unset_maximized(struct wl_client *client,
		struct wl_resource *resource) {
	struct wlr_xdg_surface_v6 *surface = wl_resource_get_user_data(resource);
	surface->toplevel_state->next.maximized = false;
}

static void xdg_toplevel_set_fullscreen(struct wl_client *client,
		struct wl_resource *resource, struct wl_resource *output_resource) {
	struct wlr_xdg_surface_v6 *surface = wl_resource_get_user_data(resource);
	surface->toplevel_state->next.fullscreen = true;
}

static void xdg_toplevel_unset_fullscreen(struct wl_client *client,
		struct wl_resource *resource) {
	struct wlr_xdg_surface_v6 *surface = wl_resource_get_user_data(resource);
	surface->toplevel_state->next.fullscreen = false;
}

static void xdg_toplevel_set_minimized(struct wl_client *client,
		struct wl_resource *resource) {
	struct wlr_xdg_surface_v6 *surface = wl_resource_get_user_data(resource);
	wl_signal_emit(&surface->events.request_minimize, surface);
}

static const struct zxdg_toplevel_v6_interface zxdg_toplevel_v6_implementation =
{
	.destroy = resource_destroy,
	.set_parent = xdg_toplevel_set_parent,
	.set_title = xdg_toplevel_set_title,
	.set_app_id = xdg_toplevel_set_app_id,
	.show_window_menu = xdg_toplevel_show_window_menu,
	.move = xdg_toplevel_move,
	.resize = xdg_toplevel_resize,
	.set_max_size = xdg_toplevel_set_max_size,
	.set_min_size = xdg_toplevel_set_min_size,
	.set_maximized = xdg_toplevel_set_maximized,
	.unset_maximized = xdg_toplevel_unset_maximized,
	.set_fullscreen = xdg_toplevel_set_fullscreen,
	.unset_fullscreen = xdg_toplevel_unset_fullscreen,
	.set_minimized = xdg_toplevel_set_minimized
};

static void xdg_surface_destroy(struct wlr_xdg_surface_v6 *surface) {
	wl_signal_emit(&surface->events.destroy, surface);
	wl_resource_set_user_data(surface->resource, NULL);
	wl_list_remove(&surface->link);
	wl_list_remove(&surface->surface_destroy_listener.link);
	wl_list_remove(&surface->surface_commit_listener.link);
	free(surface->geometry);
	free(surface->next_geometry);
	free(surface);
}

static void xdg_surface_resource_destroy(struct wl_resource *resource) {
	struct wlr_xdg_surface_v6 *surface = wl_resource_get_user_data(resource);
	if (surface != NULL) {
		xdg_surface_destroy(surface);
	}
}

static void xdg_surface_get_toplevel(struct wl_client *client,
		struct wl_resource *resource, uint32_t id) {
	struct wlr_xdg_surface_v6 *surface = wl_resource_get_user_data(resource);

	if (wlr_surface_set_role(surface->surface, wlr_desktop_xdg_toplevel_role,
			resource, ZXDG_SHELL_V6_ERROR_ROLE)) {
		return;
	}

	if (!(surface->toplevel_state =
			calloc(1, sizeof(struct wlr_xdg_toplevel_v6)))) {
		return;
	}

	surface->role = WLR_XDG_SURFACE_V6_ROLE_TOPLEVEL;

	struct wl_resource *toplevel_resource = wl_resource_create(client,
		&zxdg_toplevel_v6_interface, wl_resource_get_version(resource), id);

	wl_resource_set_implementation(toplevel_resource,
		&zxdg_toplevel_v6_implementation, surface, NULL);
	struct wl_display *display = wl_client_get_display(client);
	zxdg_surface_v6_send_configure(resource, wl_display_next_serial(display));
}

static void xdg_surface_get_popup(struct wl_client *client,
		struct wl_resource *resource, uint32_t id, struct wl_resource *parent,
		struct wl_resource *wl_positioner) {
	wlr_log(L_DEBUG, "TODO xdg surface get popup");
}

static void xdg_surface_ack_configure(struct wl_client *client,
		struct wl_resource *resource, uint32_t serial) {
	wlr_log(L_DEBUG, "TODO xdg surface ack configure");
}

static void xdg_surface_set_window_geometry(struct wl_client *client,
		struct wl_resource *resource, int32_t x, int32_t y, int32_t width,
		int32_t height) {
	struct wlr_xdg_surface_v6 *surface = wl_resource_get_user_data(resource);
	surface->has_next_geometry = true;
	surface->next_geometry->height = height;
	surface->next_geometry->width = width;
	surface->next_geometry->x = x;
	surface->next_geometry->y = y;
}

static const struct zxdg_surface_v6_interface zxdg_surface_v6_implementation = {
	.destroy = resource_destroy,
	.get_toplevel = xdg_surface_get_toplevel,
	.get_popup = xdg_surface_get_popup,
	.ack_configure = xdg_surface_ack_configure,
	.set_window_geometry = xdg_surface_set_window_geometry,
};

static void xdg_shell_create_positioner(struct wl_client *client,
		struct wl_resource *resource, uint32_t id) {
	wlr_log(L_DEBUG, "TODO: xdg shell create positioner");
}

static void handle_wlr_surface_destroyed(struct wl_listener *listener,
		void *data) {
	struct wlr_xdg_surface_v6 *xdg_surface =
		wl_container_of(listener, xdg_surface, surface_destroy_listener);
	xdg_surface_destroy(xdg_surface);
}

static void wlr_xdg_surface_v6_toplevel_committed(
		struct wlr_xdg_surface_v6 *surface) {
	surface->toplevel_state->current.maximized =
		surface->toplevel_state->next.maximized;
	surface->toplevel_state->current.fullscreen =
		surface->toplevel_state->next.fullscreen;
	surface->toplevel_state->current.resizing =
		surface->toplevel_state->next.resizing;
	surface->toplevel_state->current.activated =
		surface->toplevel_state->next.activated;

	surface->toplevel_state->current.max_width =
		surface->toplevel_state->next.max_width;
	surface->toplevel_state->current.max_height =
		surface->toplevel_state->next.max_height;

	surface->toplevel_state->current.min_width =
		surface->toplevel_state->next.min_width;
	surface->toplevel_state->current.min_height =
		surface->toplevel_state->next.min_height;
}

static void handle_wlr_surface_committed(struct wl_listener *listener,
		void *data) {

	struct wlr_xdg_surface_v6 *surface =
		wl_container_of(listener, surface, surface_commit_listener);

	if (surface->has_next_geometry) {
		surface->has_next_geometry = false;
		surface->geometry->x = surface->next_geometry->x;
		surface->geometry->y = surface->next_geometry->y;
		surface->geometry->width = surface->next_geometry->width;
		surface->geometry->height = surface->next_geometry->height;
	}

	if (surface->role == WLR_XDG_SURFACE_V6_ROLE_TOPLEVEL) {
		wlr_xdg_surface_v6_toplevel_committed(surface);
	}

	wl_signal_emit(&surface->events.commit, surface);
}

static void xdg_shell_get_xdg_surface(struct wl_client *client,
		struct wl_resource *_xdg_shell, uint32_t id,
		struct wl_resource *_surface) {
	struct wlr_xdg_shell_v6 *xdg_shell = wl_resource_get_user_data(_xdg_shell);
	struct wlr_xdg_surface_v6 *surface;
	if (!(surface = calloc(1, sizeof(struct wlr_xdg_surface_v6)))) {
		return;
	}

	if (!(surface->geometry = calloc(1, sizeof(struct wlr_box)))) {
		free(surface);
		return;
	}

	if (!(surface->next_geometry = calloc(1, sizeof(struct wlr_box)))) {
		free(surface->geometry);
		free(surface);
		return;
	}

	surface->role = WLR_XDG_SURFACE_V6_ROLE_NONE;
	surface->surface = wl_resource_get_user_data(_surface);
	surface->resource = wl_resource_create(client,
		&zxdg_surface_v6_interface, wl_resource_get_version(_xdg_shell), id);

	wl_signal_init(&surface->events.request_minimize);
	wl_signal_init(&surface->events.commit);
	wl_signal_init(&surface->events.destroy);

	wl_signal_add(&surface->surface->signals.destroy,
		&surface->surface_destroy_listener);
	surface->surface_destroy_listener.notify = handle_wlr_surface_destroyed;

	wl_signal_add(&surface->surface->signals.commit,
			&surface->surface_commit_listener);
	surface->surface_commit_listener.notify = handle_wlr_surface_committed;

	wlr_log(L_DEBUG, "new xdg_surface %p (res %p)", surface, surface->resource);
	wl_resource_set_implementation(surface->resource,
		&zxdg_surface_v6_implementation, surface, xdg_surface_resource_destroy);
	wl_list_insert(&xdg_shell->surfaces, &surface->link);
}

static void xdg_shell_pong(struct wl_client *client,
		struct wl_resource *resource, uint32_t serial) {
	wlr_log(L_DEBUG, "TODO xdg shell pong");
}

static struct zxdg_shell_v6_interface xdg_shell_impl = {
	.create_positioner = xdg_shell_create_positioner,
	.get_xdg_surface = xdg_shell_get_xdg_surface,
	.pong = xdg_shell_pong,
};

static void xdg_shell_destroy(struct wl_resource *resource) {
	wl_list_remove(wl_resource_get_link(resource));
}

static void xdg_shell_bind(struct wl_client *wl_client, void *_xdg_shell,
		uint32_t version, uint32_t id) {
	struct wlr_xdg_shell_v6 *xdg_shell = _xdg_shell;
	assert(wl_client && xdg_shell);
	if (version > 1) {
		wlr_log(L_ERROR,
			"Client requested unsupported xdg_shell_v6 version, disconnecting");
		wl_client_destroy(wl_client);
		return;
	}
	struct wl_resource *wl_resource =
		wl_resource_create( wl_client, &zxdg_shell_v6_interface, version, id);
	wl_resource_set_implementation(wl_resource, &xdg_shell_impl, xdg_shell,
		xdg_shell_destroy);
	wl_list_insert(&xdg_shell->wl_resources, wl_resource_get_link(wl_resource));
}

struct wlr_xdg_shell_v6 *wlr_xdg_shell_v6_create(struct wl_display *display) {
	struct wlr_xdg_shell_v6 *xdg_shell =
		calloc(1, sizeof(struct wlr_xdg_shell_v6));
	if (!xdg_shell) {
		return NULL;
	}
	struct wl_global *wl_global = wl_global_create(display,
		&zxdg_shell_v6_interface, 1, xdg_shell, xdg_shell_bind);
	if (!wl_global) {
		free(xdg_shell);
		return NULL;
	}
	xdg_shell->wl_global = wl_global;
	wl_list_init(&xdg_shell->wl_resources);
	wl_list_init(&xdg_shell->surfaces);
	return xdg_shell;
}

void wlr_xdg_shell_v6_destroy(struct wlr_xdg_shell_v6 *xdg_shell) {
	if (!xdg_shell) {
		return;
	}
	struct wl_resource *resource = NULL, *temp = NULL;
	wl_resource_for_each_safe(resource, temp, &xdg_shell->wl_resources) {
		struct wl_list *link = wl_resource_get_link(resource);
		wl_list_remove(link);
	}
	// TODO: destroy surfaces
	// TODO: this segfault (wl_display->registry_resource_list is not init)
	// wl_global_destroy(xdg_shell->wl_global);
	free(xdg_shell);
}
