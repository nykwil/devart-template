# Survival of the Fittest
A Gene Based Internet Trend Collage System

## Authors
- Robb Gray

## Description
The work looks to explore our obession with internet trends (memes) and celebrity, by using a gene based algorithm that emulates the same system that brings rise to them.  

Using the results of Google Trends website, the top two trends are pitted against each other to create a work for that day.

Using v8 and the ATOM api, the google image results of the two trends are download.  One trend is marked as the portrait and the "face" search criteria is clicked, the other is marked as the source and all top 10 image results are downloaded.

The algorithm starts cutting out shapes from the source images and placing them in layers in the intent to recreate the portrait.

Genetic algorithms uses image similiary as it's fitness and determine the best cut out and placement.  Slowly building toward recreating the image.

On a technology side I wanted to create an openframeworks addon that would demonstrates a generic aproach to image analysis based genetic art generation.

## Links to External Libraries

[openframeworks](http://www.google.com "openframeworks")
[ofxGALib](http://www.google.com "ofxGALib")
[v8](http://www.google.com "v8")

## Images & Videos
Results from the early prototype:

![Example Image](project_images/pearl_earing.jpg?raw=true "Example Image")

![Example Image](project_images/pearl_earing%20(5).png?raw=true "Example Image")



