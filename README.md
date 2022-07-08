# Ciallo: Next generation 2D dcc software

> Ciallo～(∠・ω< )⌒★!  anime/cartoon computer graphics.

Ciallo aims for improving 2d artist's work efficiency and painting experience.

Being different from traditional pixelmap or bezier curve based painting software, polylines/polygons are the first citizen in ciallo. They are rendered with GPU and edited based on some geometry algorithms. Quite a lot of features need to be researched and developed.

Why using polyline? Polyline can approximate any kind of curve, can honestly record stylus/tablet data (bezier curve always try to overfits). And applying the geometry algorithms on polyline is relatively easy.

Ciallo is greatly inspired by [blender grease pencil](https://docs.blender.org/manual/en/latest/grease_pencil/introduction.html). I got a lot of help from @Clément Foucault  and @Falk David  when learning about code of grease pencil system. The grease pencil is based on polylines in 3D space and render them on GPU. Though Ciallo uses polylines in lower dimension space, we can utilize the powerful geometry tools only available in 2D space. 

Here are some successful artworks drawn in grease pencil. [GPencil open project](https://cloud.blender.org/p/gallery/5b642e25bf419c1042056fc6) , [Hero](https://www.youtube.com/watch?v=pKmSdY56VtY&t=3s).

## Innovative features preview

- GPU powered brush engine
- Real time "label to fill"
- Line binding (parenting) for editing and animating

### GPU powered brush engine

In July 2022, two engines are made: _dot engine_, _articulated engine_. Dot engine evenly place texture quad along the polyline and articulated engine render the polyline as if an articulated arm. Their math/geometry behind the scene are pretty straight forward except for airbrush generated from articulated engine. It needs bachelor level of calculus to clarify the whole ideas. I've made a draft to explain the brush in Jan 2022 but it's definitely too messy to be read. I'll try to clean it up someday. If you are literally interested in how to render an airbrush stroke on a bezier curve (which takes [8 steps](https://www.techwalla.com/articles/how-to-airbrush-in-illustrator) in illustrator), have a try on the draft.

### Real time "label to fill"

Automated coloring process (wrapping up the [CGAL 2D Arrangement](https://doc.cgal.org/latest/Arrangement_on_surface_2/index.html) for users):



In animation industry, artists use labeling lines to indicate how to fill colors. In the picture above, blue means shadow and red means highlight. Ciallo utilizes these labeling lines and generate polygon data for GPU to render instead of flood fill on pixelmap. It usually takes less than 1ms on an pre-arranged data sets. If you want to know some statistic on polyline/polygon based artwork's data size (number of lines and vertices), here is the [link].

Similar feature offered by Krita is called "[colorize mask](https://docs.krita.org/en/reference_manual/tools/colorize_mask.html)". But it seems not fast enough for production and hard to edit the content on canvas since it's pixelmap. Take a look at [this video](https://www.youtube.com/watch?v=HQdx6H9BIGs) if you are interested in the problem of colorize mask.

In combination with line binding, users can animate the scene with color at great ease.

### Line binding for editing and animating

Polylines are hard to edit but bezier curves are pretty easy. So Ciallo allows users to "bind" a bezier curve upon a polyline. It'll act like this:



So the polyline keeps high frequency information (noise), meanwhile bezier curve helps to edit polyline's low frequency information (overall shapes). When it comes to animating, bezier curve can make animation like bones in 3D after binding, which lets 2D artists reuse their data in stroke level. In the future, we can pop bezier curve data into uniform buffer and calculate animations on GPU.

## Other potential usage

- Save AAA games suffering from texture loading.
- Offer diverse line and polygon rendering solution for UI frameworks.

