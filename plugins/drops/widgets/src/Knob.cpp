/* license */
#include "Knob.hpp"

START_NAMESPACE_DISTRHO

Knob::Knob(Window &parent) noexcept
    : NanoWidget(parent)
{
    loadSharedResources();
    dragging_ = false;
    has_mouse_ = false;
    value_ = 0.f;
    value_tmp_ = 0.f;
    maximum_value = 1.0f;
    minimum_value = 0.0f;
    using_log_ = false;
    labelSize = 14.0f;
    label = "label";
    margin = 4.0f;
    fontFace(NANOVG_DEJAVU_SANS_TTF);
    font_ = findFont(NANOVG_DEJAVU_SANS_TTF);
    if (font_ == -1)
    {
        fprintf(stderr, "%s", "font not found\n");
    }
    foreground_color = Color(1, 1, 1);
    background_color = Color(0, 0, 0);
    text_color = Color(1, 1, 1);
    fill_color_ = foreground_color;
}
float Knob::getValue() noexcept {
    return value_;
}
bool Knob::onMouse(const MouseEvent &ev)
{
    if (ev.button != 1)
        return false;
    has_mouse_ = true;
    if (ev.press)
    {
        if (!contains(ev.pos))
            return false;

        if ((ev.mod & kModifierShift) != 0 && using_default_)
        {
            setValue(default_value);
            tmp_value_ = value_;
            return true;
        }

        dragging_ = true;
        last_mouse_x_ = ev.pos.getX();
        last_mouse_y_ = ev.pos.getY();

        if (callback != nullptr)
            callback->knobDragStarted(this);

        return true;
    }
    else if (dragging_)
    {
        if (callback != nullptr)
            callback->knobDragFinished(this);

        dragging_ = false;
        repaint();
        return true;
    }

    has_mouse_ = false;
    return false;
}

bool Knob::onScroll(const ScrollEvent &ev)
{
    if (!contains(ev.pos))
        return false;

    const float d = (ev.mod & kModifierControl) ? 2000.0f : 200.0f;
    float value = (using_log_ ? _invlogscale(tmp_value_) : tmp_value_) + (float(maximum_value - minimum_value) / d * 10.f * ev.delta.getY());

    if (using_log_)
        value = _logscale(value);

    if (value < minimum_value)
    {
        tmp_value_ = value = minimum_value;
    }
    else if (value > maximum_value)
    {
        tmp_value_ = value = maximum_value;
    }
    setValue(value);
    return true;
}

bool Knob::onMotion(const MotionEvent &ev)
{
    if (contains(ev.pos) && !has_mouse_)
    {
        has_mouse_ = true;
        repaint();
    }
    if (!contains(ev.pos) && !dragging_)
    {
        has_mouse_ = false;
        repaint();
    }
    if (!dragging_)
        return false;

    float d, value = 0.0f;
    const int movY = last_mouse_y_ - ev.pos.getY();
    d = (ev.mod & kModifierControl) ? 2000.0f : 200.0f;
    //    printf("d = %f\n",d);
    value = (using_log_ ? _invlogscale(tmp_value_) : tmp_value_) + (float(maximum_value - minimum_value) / d * float(movY));
    //  printf("value %f\n", value);

    if (using_log_)
        value = _logscale(value);

    if (value < minimum_value)
    {
        tmp_value_ = value = minimum_value;
    }
    else if (value > maximum_value)
    {
        tmp_value_ = value = maximum_value;
    }
   
    setValue(value);

    last_mouse_x_ = ev.pos.getX();
    last_mouse_y_ = ev.pos.getY();

    return true;
}

void Knob::onNanoDisplay()
{
    const float height = getHeight();
    const float width = getWidth();

    const float stroke_width = 8.0f;

    // measure string
    fontFaceId(font_);
    fontSize(labelSize);
    Rectangle<float> bounds;
    textBounds(0.f, 0.f, label.c_str(), NULL, bounds);
    const float label_width = bounds.getWidth();
    const float label_height = bounds.getHeight();
    // label
    const float label_x = width / 2.0f; //- label_width / 2.0f;
    const float label_y = height - label_height;
    const float radius = (height - label_height - margin) / 2.0f;
    const float center_x = (width / 2.f);
    const float center_y = radius;
    beginPath();
    fillColor(text_color);
    textAlign(ALIGN_CENTER);
    text(label_x, label_y, label.c_str(), NULL);
    closePath();

    //Gauge (empty)
    beginPath();
    strokeWidth(stroke_width);
    strokeColor(background_color);
    arc(center_x, center_y, radius - stroke_width, 0.75f * M_PI, 0.25f * M_PI, NanoVG::Winding::CW);
    stroke();
    closePath();

    //Gauge (value)
    beginPath();
    strokeWidth(stroke_width);
    if (has_mouse_)
    {
        fill_color_ = highlite_color;
    }
    else
    {
        fill_color_ = foreground_color;
    }

    strokeColor(fill_color_);
    arc(center_x, center_y, radius - stroke_width, 0.75f * M_PI, (0.75f + 1.5f * value_) * M_PI, NanoVG::Winding::CW);
    stroke();
    closePath();

    // label
    // find center
    float centerY = getWidth() / 2.0f;
}

void Knob::setValue(float val) noexcept
{
    value_ = std::max(minimum_value, std::min(val, maximum_value));
    tmp_value_ = value_;
    callback->knobValueChanged(this, value_);
}

float Knob::_logscale(float value) const
{
    const float b = std::log(maximum_value / minimum_value) / (maximum_value - minimum_value);
    const float a = maximum_value / std::exp(maximum_value * b);
    return a * std::exp(b * value);
}

float Knob::_invlogscale(float value) const
{
    const float b = std::log(maximum_value / minimum_value) / (maximum_value - minimum_value);
    const float a = maximum_value / std::exp(maximum_value * b);
    return std::log(value / a) / b;
}

void Knob::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO
