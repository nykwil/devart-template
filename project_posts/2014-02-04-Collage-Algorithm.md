Using opencv contour finder to generated blobs that the program would cut out.

The genome contains a variable for a threshold.  The image is converted into a threshold image using that value (2 colour split on a specific brightness).  This means that each genome has significant control over how the image is cut into pieces, thus creating a lot of variety.

http://docs.opencv.org/doc/tutorials/imgproc/threshold/threshold.html

![Example Image](http://docs.opencv.org/_images/Threshold_Tutorial_Theory_Example.jpg?raw=true "Example Image")
