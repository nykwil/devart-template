Image Comparsion

Image comparsion is probably the most expensive and has a huge impact on the results and fitness.

I settled on something simple which was just down scaling the image and using a per pixel text.

For fitness we have to get the distnace of two colours, for colour I tried:
- CIE Delta E 1976
- JND: ~2.3
- CIE Delta E 1994
- CIE Delta E 2000

Setted on visible brightness which gave a 1D results and simple to calculate distance on

Further research has some more optimized algorithms to experiment with:

Gausinan blurring images before comparison
Sobel edge detection to identify the most significant elements of the image (contours of the face, eyes, etc)

GPU for comparison

http://docs.opencv.org/doc/tutorials/gpu/gpu-basics-similarity/gpu-basics-similarity.html
http://opencvexamples.blogspot.com/2013/10/sobel-edge-detection.html#.UzReb_ldXTc
