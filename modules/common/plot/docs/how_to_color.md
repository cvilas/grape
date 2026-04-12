# How to Generate Random Colors Programmatically

*Posted on December 9, 2009*  
*By Martin Leitner-Ankerl*

Creating random colors is actually more difficult than it seems. The randomness itself is easy, but aesthetically pleasing randomness is more difficult.

For a little project at work I needed to automatically generate multiple background colors with the following properties:

- Text over the colored background should be easily readable
- Colors should be very distinct
- The number of required colors is not initially known

## Naïve Approach

The first and simplest approach is to create random colors by simply using a random number between `[0, 256[` for the R, G, B values.

```ruby
# generates HTML code for 26 background colors given R, G, B values.
def gen_html
  ('A'..'Z').each do |c|
    r, g, b = yield
    printf "<span style=\"background-color:#%02x%02x%02x; padding:5px; -moz-border-radius:3px; -webkit-border-radius:3px;\">#{c}</span> ", r, g, b
  end
end

# naive approach: generate purely random colors
gen_html { [rand(256), rand(256), rand(256)] }
```

The generated output looks like this:

<span style="background-color:#3b82f6; color:white; padding:4px 8px; border-radius:4px;">A</span>
<span style="background-color:#111827; color:white; padding:4px 8px; border-radius:4px;">B</span>
<span style="background-color:#f59e0b; color:black; padding:4px 8px; border-radius:4px;">C</span>
<span style="background-color:#10b981; color:black; padding:4px 8px; border-radius:4px;">D</span>
<span style="background-color:#ef4444; color:white; padding:4px 8px; border-radius:4px;">E</span>
<span style="background-color:#8b5cf6; color:white; padding:4px 8px; border-radius:4px;">F</span>
<span style="background-color:#06b6d4; color:black; padding:4px 8px; border-radius:4px;">G</span>
<span style="background-color:#84cc16; color:black; padding:4px 8px; border-radius:4px;">H</span>
<span style="background-color:#ec4899; color:white; padding:4px 8px; border-radius:4px;">I</span>
<span style="background-color:#f97316; color:black; padding:4px 8px; border-radius:4px;">J</span>
<span style="background-color:#6366f1; color:white; padding:4px 8px; border-radius:4px;">K</span>
<span style="background-color:#14b8a6; color:black; padding:4px 8px; border-radius:4px;">L</span>

As you can see this is quite suboptimal. Some letters are hard to read because the background is too dark, and other colors look very similar.

## Using HSV Color Space

Let’s fix the too dark / too bright problem first. A convenient way to do this is to not use the RGB color space, but [HSV (*Hue, Saturation, Value*)](https://en.wikipedia.org/wiki/HSL_and_HSV).

Here you get equally bright and colorful colors by using a fixed value for saturation and value, and just modifying the hue.

Based on the description of [HSV-to-RGB conversion](https://en.wikipedia.org/wiki/HSL_and_HSV#Converting_to_RGB), the author implemented this converter:

```ruby
# HSV values in [0..1[
# returns [r, g, b] values from 0 to 255
def hsv_to_rgb(h, s, v)
  h_i = (h*6).to_i
  f = h*6 - h_i
  p = v * (1 - s)
  q = v * (1 - f*s)
  t = v * (1 - (1 - f) * s)
  r, g, b = v, t, p if h_i==0
  r, g, b = q, v, p if h_i==1
  r, g, b = p, v, t if h_i==2
  r, g, b = p, q, v if h_i==3
  r, g, b = t, p, v if h_i==4
  r, g, b = v, p, q if h_i==5
  [(r*256).to_i, (g*256).to_i, (b*256).to_i]
end
```

Using the generator and fixed values for saturation and value:

```ruby
# using HSV with variable hue
gen_html { hsv_to_rgb(rand, 0.5, 0.95) }
```

This returns something like this:

<span style="background-color:#f28c8c; color:black; padding:4px 8px; border-radius:4px;">A</span>
<span style="background-color:#f2c68c; color:black; padding:4px 8px; border-radius:4px;">B</span>
<span style="background-color:#e7f28c; color:black; padding:4px 8px; border-radius:4px;">C</span>
<span style="background-color:#acf28c; color:black; padding:4px 8px; border-radius:4px;">D</span>
<span style="background-color:#8cf2b3; color:black; padding:4px 8px; border-radius:4px;">E</span>
<span style="background-color:#8cf2ed; color:black; padding:4px 8px; border-radius:4px;">F</span>
<span style="background-color:#8cbaf2; color:black; padding:4px 8px; border-radius:4px;">G</span>
<span style="background-color:#a28cf2; color:white; padding:4px 8px; border-radius:4px;">H</span>
<span style="background-color:#dd8cf2; color:black; padding:4px 8px; border-radius:4px;">I</span>
<span style="background-color:#f28cd1; color:black; padding:4px 8px; border-radius:4px;">J</span>

Much better. The text is easily readable, and all colors have a similar brightness. Unfortunately, since we have limited ourselves to fewer colors now, the difference between randomly generated colors is even less than in the first approach.

## Golden Ratio

Using just `rand()` to choose different values for hue does not lead to a good use of the whole color spectrum. It simply is too random.

The author notes that random values often end up clustered too closely together, which is not what we want.

A much better approach is to use the [**golden ratio**](https://en.wikipedia.org/wiki/Golden_ratio) for spacing. The key idea is:

> Just add `1/Φ` and modulo `1` for each subsequent color.

```ruby
# use golden ratio
golden_ratio_conjugate = 0.618033988749895
h = rand # use random start value
gen_html {
  h += golden_ratio_conjugate
  h %= 1
  hsv_to_rgb(h, 0.5, 0.95)
}
```

## Final Result

The final output gives colors that are spaced much more evenly across the hue spectrum:

<span style="background-color:#f29f8c; color:black; padding:4px 8px; border-radius:4px;">A</span>
<span style="background-color:#8cb3f2; color:black; padding:4px 8px; border-radius:4px;">B</span>
<span style="background-color:#c7f28c; color:black; padding:4px 8px; border-radius:4px;">C</span>
<span style="background-color:#f28ce0; color:black; padding:4px 8px; border-radius:4px;">D</span>
<span style="background-color:#8cf2da; color:black; padding:4px 8px; border-radius:4px;">E</span>
<span style="background-color:#f2d48c; color:black; padding:4px 8px; border-radius:4px;">F</span>
<span style="background-color:#9a8cf2; color:white; padding:4px 8px; border-radius:4px;">G</span>
<span style="background-color:#8cf2a6; color:black; padding:4px 8px; border-radius:4px;">H</span>
<span style="background-color:#f28c94; color:black; padding:4px 8px; border-radius:4px;">I</span>
<span style="background-color:#8cd1f2; color:black; padding:4px 8px; border-radius:4px;">J</span>
<span style="background-color:#edf28c; color:black; padding:4px 8px; border-radius:4px;">K</span>
<span style="background-color:#d68cf2; color:black; padding:4px 8px; border-radius:4px;">L</span>

You can see that the first few values are very different, and the difference decreases as more colors are added. Still, this is usually good enough in practice.

## More Color Variants

### `s = 0.99, v = 0.99`

<span style="background-color:#fc1c1c; color:white; padding:4px 8px; border-radius:4px;">A</span>
<span style="background-color:#1cfc63; color:black; padding:4px 8px; border-radius:4px;">B</span>
<span style="background-color:#1c46fc; color:white; padding:4px 8px; border-radius:4px;">C</span>
<span style="background-color:#e11cfc; color:white; padding:4px 8px; border-radius:4px;">D</span>

### `s = 0.25, v = 0.8`

<span style="background-color:#cca3a3; color:black; padding:4px 8px; border-radius:4px;">A</span>
<span style="background-color:#a3ccb8; color:black; padding:4px 8px; border-radius:4px;">B</span>
<span style="background-color:#adb3cc; color:black; padding:4px 8px; border-radius:4px;">C</span>
<span style="background-color:#c9a3cc; color:black; padding:4px 8px; border-radius:4px;">D</span>

### `s = 0.3, v = 0.99`

<span style="background-color:#fcb1b1; color:black; padding:4px 8px; border-radius:4px;">A</span>
<span style="background-color:#b1fcd2; color:black; padding:4px 8px; border-radius:4px;">B</span>
<span style="background-color:#b1c2fc; color:black; padding:4px 8px; border-radius:4px;">C</span>
<span style="background-color:#f3b1fc; color:black; padding:4px 8px; border-radius:4px;">D</span>

## Takeaway

A good way to generate visually distinct random colors is:

1. Work in **HSV** instead of RGB
2. Keep **saturation** and **value** fixed
3. Advance **hue** by the **golden ratio conjugate** each time

This produces colors that are bright, readable, and well distributed.

## Reference

Martin Leitner-Ankerl, [*How to Generate Random Colors Programmatically*](https://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/).