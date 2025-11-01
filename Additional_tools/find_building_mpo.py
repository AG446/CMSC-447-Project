
""""
Reads in a picture, needs to be a png in black and white with whatever object you want to get the outline of need to have a red pixel to indicate the starting point, preferbely at a corner, green pixels for important points like corner, curves, etc, and blue pixels to trace the border

Takes the image and finds all the mpo objects marked by the green and red pixels and stores all the mpo coordinates for each mpo, this is stores in a 2d list

There is a sanity check that when it finds the mpo coords it will change the pixles to orange and shows the changes made to verify that the coords are found right

Reccommendation:
- instal gimp and use gimp to edit the png with the red, green, and blue pixels,
- only red and green pixels need to be very accurate the
- blue outline just needs to connect the red and green pixels so they dont need to be very accurate
- start by outline in blue first then placing red and green dots on the outline,
- keep point to a minuimum, no more then 38 since it going to be a pain to input
"""
from collections import deque
from PIL import Image
IMAGE_NAME = "map.png"
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
            #finds a red pixel and ingores whitish colors
            if g < color_threshold and b < color_threshold:
                if r > color_threshold and r > g and r > b:
                    mpo_cords.append((x,y))
                    return
            #finds a green pixel and ingores whitish colors
            elif r < color_threshold and b < color_threshold:
                if  g > color_threshold and g > r and g > b:
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
                            # Check if the neighbor is a valid blue, red, or green pixels and ingores whitish pixels
                            if g < color_threshold and b < color_threshold:
                                if is_red:
                                    queue.append((next_x, next_y))
                                    visited.add((next_x, next_y))
                            elif r < color_threshold and g < color_threshold:
                                if is_blue:
                                    queue.append((next_x, next_y))
                                    visited.add((next_x, next_y))
                            elif b < color_threshold and r < color_threshold:
                                if is_green:
                                    queue.append((next_x, next_y))
                                    visited.add((next_x, next_y))
    return

if __name__ == "__main__":
    image_path = IMAGE_NAME
    color_threshold = 150
    try:
        # Open the image file
        img = Image.open(image_path).convert('RGB')
        width, height = img.size
        #gets the pixles from the image
        pixels = img.load()
        #stores all the mpo found, this is a 2d list
        mpo_list =[]


        # Iterate over all pixels
        for i in range(width):
            for j in range(height):

                # Get the RGB tuple of the current pixel
                r, g, b = pixels[i, j]

                #check to make sure white is ingnored
                if g < color_threshold and b < color_threshold:
                     # Check if the pixel is predominantly red
                    if r > color_threshold and r > g and r > b:
                        curr_cord = (i , j)
                        #array that stores the coords for each mpo
                        #print(curr_cord) #for testing
                        mpo_cords = []
                        mpo_cords.append(curr_cord)
                        find_blue(pixels, color_threshold, width, height, mpo_cords, curr_cord)
                        mpo_list.append(mpo_cords)
                        #print(len(mpo_cords)) #for testing
        #only used for a santiy check and making sure the right pixels were found
        """
        for i in range(len(mpo_list)):
            for j in range(len(mpo_list[i])):
                pixels[mpo_list[i][j]] =(255,165,0) #used to change red and green to orange

        #used dislay the green and red pixels turned orange, only for testing to see if it worked properly
        img.save("orange", format="png")
        img.show()
        """
    except FileNotFoundError:
        print(f"Error: The file at {image_path} was not found.")
    except Exception as e:
        print(f"An error occurred: {e}")
