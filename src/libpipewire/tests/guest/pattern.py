# Copyright (C) 2026 Lubos Dolezel
# Fullscreen pattern: four quadrants red, green, blue, white, plus a small square in the centre that
# blinks so that the compositor keeps producing damage (screen casts deliver frames on damage).
import gi
for v in ("4.0", "3.0"):
    try:
        gi.require_version("Gtk", v); break
    except ValueError:
        pass
from gi.repository import Gtk, GLib
state = {"on": False}
def paint(cr, w, h):
    for i, c in enumerate([(1,0,0),(0,1,0),(0,0,1),(1,1,1)]):
        cr.set_source_rgb(*c); cr.rectangle((i%2)*w/2, (i//2)*h/2, w/2, h/2); cr.fill()
    v = 1 if state["on"] else 0
    cr.set_source_rgb(v, v, v); cr.rectangle(w/2-20, h/2-20, 40, 40); cr.fill()
if Gtk.get_major_version() == 4:
    def draw(area, cr, w, h): paint(cr, w, h)
    def act(app):
        win = Gtk.ApplicationWindow(application=app)
        a = Gtk.DrawingArea(); a.set_draw_func(draw); win.set_child(a)
        def tick():
            state["on"] = not state["on"]; a.queue_draw(); return True
        GLib.timeout_add(100, tick)
        win.fullscreen(); win.present()
    app = Gtk.Application(application_id="x.pwt.pattern"); app.connect("activate", act); app.run(None)
else:
    win = Gtk.Window(); a = Gtk.DrawingArea()
    def draw(wd, cr): paint(cr, wd.get_allocated_width(), wd.get_allocated_height())
    a.connect("draw", draw); win.add(a); win.fullscreen(); win.connect("destroy", Gtk.main_quit); win.show_all()
    def tick():
        state["on"] = not state["on"]; a.queue_draw(); return True
    GLib.timeout_add(100, tick); Gtk.main()
