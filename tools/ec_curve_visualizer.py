# /// script
# requires-python = ">=3.10"
# dependencies = ["matplotlib>=3.8,<4"]
# ///
"""Plot the lookup table used by the DMK EC travel curve."""

from math import isfinite

POINT_LIMITS = ((2, 94), (4, 96), (6, 98))
DEFAULTS = (15, 50, 80, 40, 10)
SIGNAL_GRID = range(0, 1001, 8)


def build_lut(p1, p2, p3, depth_dmm, smooth_tenths):
    if any(not low <= point <= high
           for point, (low, high) in zip((p1, p2, p3), POINT_LIMITS)):
        raise ValueError("ADC point outside its allowed range.")
    if not 31 <= depth_dmm <= 45 or not 0 <= smooth_tenths <= 10:
        raise ValueError("Maximum travel or smoothing outside its allowed range.")

    x = (0, p1 * 10, p2 * 10, p3 * 10, 1000)
    y = (0, 250, 500, 750, depth_dmm * 25)
    h = [right - left for left, right in zip(x, x[1:])]
    if min(h) < 20:
        raise ValueError("ADC points must increase by at least 2 percentage points.")
    d = [(y[i + 1] - y[i]) / h[i] for i in range(4)]

    def edge(h0, h1, d0, d1):
        return max(0, ((2 * h0 + h1) * d0 - h0 * d1) / (h0 + h1))

    m = [edge(h[0], h[1], d[0], d[1])]
    for i in range(1, 4):
        left, right = 2 * h[i] + h[i - 1], h[i] + 2 * h[i - 1]
        m.append((left + right) / (left / d[i - 1] + right / d[i]))
    m.append(edge(h[3], h[2], d[3], d[2]))

    lut = []
    segment = 0
    for signal in SIGNAL_GRID:
        if segment < 3 and signal >= x[segment + 1]:
            segment += 1
        u = (signal - x[segment]) / h[segment]
        a, b = y[segment], y[segment + 1]
        linear = a + u * (b - a)
        cubic = a + u * u * (3 - 2 * u) * (b - a) + h[segment] * u * (1 - u) * (
            (1 - u) * m[segment] - u * m[segment + 1]
        )
        lut.append(max(lut[-1] if lut else 0, int(linear + smooth_tenths / 10 * (cubic - linear) + 0.5)))
    if any(right - left > 32 for left, right in zip(lut[7:], lut[8:])):
        raise ValueError("Curve too steep above the reset zone (maximum 4x).")
    return lut


def main():
    import matplotlib.pyplot as plt
    from matplotlib.widgets import Button, Slider, TextBox

    fig, ax = plt.subplots(figsize=(9, 7))
    fig.subplots_adjust(bottom=0.40, left=0.24)
    ax.set(title="DMK travel curve", xlabel="Travel (mm)", ylabel="Normalized ADC signal (%)",
           xlim=(0, 4.6), ylim=(0, 100))
    ax.grid(alpha=0.3)
    line, = ax.plot([], [], label="Firmware lookup table")
    points, = ax.plot([], [], "o", label="Calibration points")
    ax.legend(loc="lower right")
    status = fig.text(0.06, 0.065, "")

    specs = (
        ("ADC at 1 mm (%)", *POINT_LIMITS[0], DEFAULTS[0], 1, "%0.0f"),
        ("ADC at 2 mm (%)", *POINT_LIMITS[1], DEFAULTS[1], 1, "%0.0f"),
        ("ADC at 3 mm (%)", *POINT_LIMITS[2], DEFAULTS[2], 1, "%0.0f"),
        ("Maximum travel (mm)", 3.1, 4.5, DEFAULTS[3] / 10, 0.1, "%0.1f"),
        ("Smoothing", 0.0, 1.0, DEFAULTS[4] / 10, 0.1, "%0.1f"),
    )
    sliders = [
        Slider(fig.add_axes((0.25, 0.30 - i * 0.047, 0.52, 0.025)),
               label, low, high, valinit=initial, valstep=step, valfmt=fmt)
        for i, (label, low, high, initial, step, fmt) in enumerate(specs)
    ]
    fields = []

    def bind_input(slider, field, low, high, step, fmt):
        def update_field(value):
            formatted = fmt % value
            if field.text != formatted:
                # Programmatic updates must not submit the field again.
                field.eventson = False
                try:
                    field.set_val(formatted)
                finally:
                    field.eventson = True

        def submit(text):
            try:
                value = float(text.strip().replace(",", "."))
            except ValueError:
                update_field(slider.val)
                return
            if not isfinite(value):
                update_field(slider.val)
                return
            value = min(high, max(low, value))
            value = min(high, low + round((value - low) / step) * step)
            if slider.val != value:
                slider.set_val(value)
            update_field(slider.val)

        slider.valtext.set_visible(False)
        slider.on_changed(update_field)
        field.on_submit(submit)

    for i, (slider, (_, low, high, initial, step, fmt)) in enumerate(zip(sliders, specs)):
        field = TextBox(fig.add_axes((0.80, 0.295 - i * 0.047, 0.11, 0.035)),
                        "", initial=fmt % initial, textalignment="center")
        bind_input(slider, field, low, high, step, fmt)
        fields.append(field)

    def redraw(_=None):
        p1, p2, p3 = (round(slider.val) for slider in sliders[:3])
        depth, smooth = (round(slider.val * 10) for slider in sliders[3:])
        points.set_data((0, 1, 2, 3, depth / 10), (0, p1, p2, p3, 100))
        try:
            lut = build_lut(p1, p2, p3, depth, smooth)
        except ValueError as exc:
            line.set_data((0, 4), (0, 100))
            status.set_text(f"{exc}\nFirmware falls back to the standard response.")
            status.set_color("firebrick")
        else:
            line.set_data([travel / 250 for travel in lut], [signal / 10 for signal in SIGNAL_GRID])
            status.set_text("Valid curve. Copy these five values into VIA.")
            status.set_color("darkgreen")
        fig.canvas.draw_idle()

    for slider in sliders:
        slider.on_changed(redraw)
    reset = Button(fig.add_axes((0.82, 0.045, 0.11, 0.04)), "Reset")

    def reset_defaults(_):
        for slider in sliders:
            slider.reset()

    reset.on_clicked(reset_defaults)
    redraw()
    plt.show()


if __name__ == "__main__":
    main()
