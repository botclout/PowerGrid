# Overview of Changes from 8/3 to 16/3 #
This document shows what the major changes are since march 8th. It mainly shows my view on
the project and decisions that I made of which I think are good practise to maintain. I hope
that you agree with me on the following points and that you will also follow them, in order to
improve the software quality and in the meantime iron out the imperfections from the project.

Chronio

### Changes in Mapping and Mapped Data Format ### 

Map data has become more accurate and complete. It is now possible to maintain references 
to core RS data by cutting out the middleman (SceneObject). This has as a benefit that 
access times for map data are probably faster, and less memory is used because existing 
objects are cached instead of creating new ones. It also holds basically all information
available to RSBot, instead of only the part that SceneObject contains. 

If my hunch is correct, RSBot automatically instantiates RSObjects when they are loaded in 
Runescape, so mapping speed should have increased dramatically, too (SceneObjects are only 
instantiated per request, making retrieval much slower than when using RSObjects). Also, 
SceneEntities.getLoaded() is a very inefficient method to use (look it up, you'll see).

The Map data itself is now stored in a WorldMap instance rather than a Grid (Grid is removed 
since it was not used anymore anyway, and I thought it was ugly once I used reflection for 
categorising GameObjects). The now current WorldMap stores an actual Map from Point to Field, 
where a Field contains both the collision flag and the RSGround objects for that Point. These 
can be retrieved separately, or combined and instantiated as a GameObject, containing the 
information from both. GameObjects are immutable, and as such provide an easy way of caching 
specific data when desired.


### Changes in JavaDoc Style ### 

From now on, package info is documented in a file called "package.html" inside each package.
This package info is automatically parsed and included in de documentation by the Javadoc 
Tool. More specifically, everything in between <body> and </body> will be parsed, including
@see, @since, and {@link name} tags. The first line of text is shown as the package description
in the overview. Also, @author tags are now included in the Javadoc, to show plugin developers 
who made each part of PowerGrid, so that they can complain to the correct person (saves a lot of 
frustration, since nobody can explain a piece of code better than its author).


### Changes in Testing Policy ###

From now on, tests will be written for each class that is testable with a Unit test. For this 
purpose, JUnit will be used, possibly in combination with Mockito. I am willing to explain Mockito
and its functions and benefits at any time, just say the word. Before each commit, it is advised 
to run the Test suite in its entirety, to ensure each class behaves as expected. Because of this,
it is important to keep test cases quick and small and prevent any redundant setup. Having test 
cases for each class also allows us to check if changes in a certain class are code-breaking in 
another.

Besides Unit tests, AcceptanceTests (or StoryTests) will be implemented for each common PowerGrid 
operation that allows such an AcceptanceTest to be executed. Mockito can be used to isolate the
testing environment.