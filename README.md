# Ciallo: Next generation 2D dcc software

> Ciallo～(∠・ω< )⌒★!  Anime/cartoon computer graphics.

Ciallo aims for improving 2d artist's work efficiency and painting experience.

Being different from traditional pixelmap or bezier curve based painting software, polylines/polygons are the first citizen in ciallo. They are rendered with GPU and edited based on some geometry algorithms. Quite a lot of features need to be researched and developed.

Why using polyline? Polyline can approximate any kind of curve, can honestly record stylus/tablet data (bezier curve always try to overfits). And applying geometry algorithms on polyline is relatively easy.

Ciallo is greatly inspired by [blender grease pencil](https://docs.blender.org/manual/en/latest/grease_pencil/introduction.html). I got a lot of help from @Clément Foucault  and @Falk David  when learning the code of grease pencil system. The grease pencil is based on polylines in 3D space and render them on GPU. Ciallo discard one space dimension, which make it possible to utilize some powerful geometry tools only available on curve in 2D space like 2D arrangements, 2D generalized winding number and 2D Envelopes.

Here are some successful artworks drawn with polyline method (in blender): [GPencil open project](https://cloud.blender.org/p/gallery/5b642e25bf419c1042056fc6) and several statistics on the number of vertices and strokes from those artworks.

## Features preview

- Real time "label to fill"
- Brush engines powered by GPU
- Line binding (parenting) for stroke editing and animating

### Real time "label to fill"

Automate coloring process (wrapping up the [CGAL 2D Arrangement](https://doc.cgal.org/latest/Arrangement_on_surface_2/index.html) for users):



In animation industry, artists use labeling lines to indicate how to fill colors. In the picture above, blue strokes means shadow and red strokes means highlight. Ciallo will utilize these labeling strokes and query polygons enclose those labels (these polygons are called face). Then send the polygon data to GPU for rendering instead of flood fill on pixelmap. It usually takes less than 1ms on a pre-arranged data set. If you want to know more about statistic on polyline/polygon based artwork's data size (number of lines and vertices), here is the [link].

Similar feature offered by Krita is called "[colorize mask](https://docs.krita.org/en/reference_manual/tools/colorize_mask.html)". But it seems not fast enough for production and hard to edit the content on canvas since it's based on pixelmap. Take a look at [this video](https://www.youtube.com/watch?v=HQdx6H9BIGs) if you are interested in the problem of colorize mask.

In combination with line binding, users can animate their artworks at great ease. When modifying the label strokes and strokes enclose them, faces generated from strokes are updated automatically.

### Brush engines powered by GPU 

Though inspired by blender grease pencil, the rendering method in Ciallo is quite different from grease pencil. The new method fix some fatal drawbacks and will aim for flexibility instead of performance.

In July 2022, two engines are made: _Equidistant Dot_ and _Articulated Line_. As the name imply, _equidistant dot_ evenly place texture quad along the polyline. _Articulated line_ render a polyline as if an articulated arm. Here are some comparisons between them:

| Features                | Equidistant Dot                                              | Articulated Line                                             |
| ----------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| Great performance       | Yes, better than methods on CPU.                             | Yes, better than Equidistant Dot in theory.                  |
| Customization by users  | Easy. Procedural texture is not mandatory.                   | Hard. Procedural texture is mandatory. Need experience in shader development or shader graph node system. |
| Robustness to ill cases | One ill case would be pretty common (unevenly distributed vertices) and it could hit hard on rendering performance. Need help from editor to avoid it. | Better than Equidistant Dot. Pretty few ill cases I've found and they rarely happen in practice. |
| Limitations on vertices | Total amount of vertices input are limited be *maximum local workgroup size* (1024) in compute shader. Total amount of dots generated are limited by buffer size set by developers. | No limits on regular usage.                                  |

Their computes about geometry are pretty straight forward except for airbrush generated from articulated line. It needs calculus to clarify the whole idea about solving "the joint problem". I've made a draft to explain the brush in Jan. 2022 but it's definitely too messy to read. I'll try to clean it up someday. Unless you are ultra interested in rendering an airbrush stroke on a bezier curve (which takes [8 steps in illustrator](https://www.techwalla.com/articles/how-to-airbrush-in-illustrator)), I do not recommend to read it.

### Line binding for editing and animating

Polylines are hard to edit but bezier curves are pretty easy. So Ciallo allows users to "bind" a bezier curve upon a polyline. It'll act like this:



So the polyline keeps high frequency information (noise), meanwhile bezier curve helps to edit polyline's low frequency information (overall shapes). When it comes to animating, bezier curve can make animation like bones in 3D after binding, which lets 2D artists reuse their data in stroke level. In the future, we can pop bezier curve data into uniform buffer and calculate animations on GPU.

## Other potential usage

- Save AAA games suffering from texture loading.
- Offer diverse line and polygon rendering solution for UI frameworks.

