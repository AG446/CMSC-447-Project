
""""
Reads in a picture, needs to be a png in black and white with whatever object you want to get the border of having only 1 red pixel at a corner and blue pixels tracing the outline of the object, the blue pixels need to connect to the red pixels on both sides.

Takes the image and finds all the mpo objects marked by the blue and red pixels and stores all the mpo coordinates for each mpo, this is stores in a 2d list

There is a sanity check that when it finds the mpo coords it will change the pixles to white and shows the chnages made to verify that the coords are found right

Reccommendation: its easier to edits the image in canva by tracing the objects in blue with weight size of 1 then change the color to red and weight size to 2 and put a red dot anywhere in the blue trace, this will make 2 pixels red so you will have to edit the image first to get rid of the second red pixel, the blue lines must not overlap other wise the mpo coords will be off.
"""
from collections import deque
from PIL import Image

# used to find the blue lines that connect the green dot(corners of the buildings)
#only used to find 1 buulding at a time
def find_blue(pixles, color_threshold, width, height, mpo_cords, start_cord):
    # Store visited pixels to prevent infinite loops
    visited = set()
    # Use a queue for a Breadth-First Search (BFS) that has the starting cords in the queue
    queue = deque([start_cord])
    #used to remove the starting cord from vistited after a certin pixel amount it checked
    #makes sure the code finds the red pixel again since start and end is the same pixel
    counter = 0
    #continues until the queue is empty
    while queue:
        #gets current corrd we are checking
        x, y = queue.popleft()

        if (x,y) != start_cord:
            r, g, b = pixels[x, y]
            if r > color_threshold and r > g and r > b:
                mpo_cords.append((x,y))
                return
            elif  g > color_threshold and g > r and g > b:
                mpo_cords.append((x,y))
        #add cord to visited so we dont check again
        visited.add((x, y))
        counter +=1
        #removes starting cord from visited after 10 pixles are checked
        if counter == 10:
            visited.remove(start_cord)

        # Check all 8 neighboring pixels
        for dx in [-1, 0, 1]:
            for dy in [-1, 0, 1]:
                if dx != 0 or dy != 0:
                    next_x = x + dx
                    next_y = y + dy
                    # Boundary check
                    if 0 <= next_x < width and 0 <= next_y < height:
                        #checks if the neighboring pixels have been visited
                        if (next_x, next_y) not in visited:
                            # Get the color of the neighboring pixel
                            r, g, b = pixels[next_x, next_y]
                            is_blue = b > color_threshold and b > g and b > r
                            is_red = r > color_threshold and r > g and r > b
                            is_green = g > color_threshold and g > r and g > b
                            # Check if the neighbor is a valid blue or red pixel
                            if is_blue or is_red or is_green:
                                queue.append((next_x, next_y))
    return

if __name__ == "__main__":
    image_path = 'test1.png'
    color_threshold = 100
    try:
        # Open the image file
        img = Image.open(image_path).convert('RGB')
        width, height = img.size
        #gets the pixles from the image
        pixels = img.load()
        #stores all the mpo found, this is a 2d list
        mpo_list =[]

        """
        once you find the second red pixels you can copy this code and just replace x and y with the cords, thie will change the second red pixel to blue
        pixels[x,y] = (0, 0, 255)

        """
        #this section is specificaly for the test1.png so that it works
        pixels[876,480] = (0, 0, 255)
        pixels[922,514] = (0, 0, 255)
        pixels[973,496] = (0, 0, 255)
        # Iterate over all pixels
        for i in range(width):
            for j in range(height):

                # Get the RGB tuple of the current pixel
                r, g, b = pixels[i, j]
                # Check if the pixel is predominantly red

                if r > color_threshold and r > g and r > b:
                    curr_cord = (i , j)
                    #use this to 1st find all the second red pixles for the mpo then commet it out if you want the second time
                    print(curr_cord)
                    #array of tuples that holds the cords of the building mpo(map polygram object)
                    #array that stores the coords for each mpo
                    mpo_cords = []
                    mpo_cords.append(curr_cord)
                    find_blue(pixels, color_threshold, width, height, mpo_cords, curr_cord)
                    mpo_list.append(mpo_cords)


        for i in range(len(mpo_list)):
            for j in range(len(mpo_list[i])):
                pixels[mpo_list[i][j]] = (255, 255, 255) #used to change red and blue to white

        #used dislay the green pixels turned black, only for testing to see if ti worked properly
        img.save("black", format="png")
        img.show()

    except FileNotFoundError:
        print(f"Error: The file at {image_path} was not found.")
    except Exception as e:
        print(f"An error occurred: {e}")
