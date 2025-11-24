
""""
same as find nodes but only finds green pixels and transforms them into real wprld coordinates
"""
from collections import deque
from PIL import Image, ImageDraw, ImageFont
"""
IMAGE_NAME = "LIB_area1.png"
#you need to already know the real location of the pixel, the conversion will be based on this so make it as accurate as possible
TOP_RIGHT_PIX = (1157,187)
TOP_RIGHT_REAL = (-76.70906,39.25624)

BOTTOM_LEFT_PIX = (44, 589)
BOTTOM_LEFT_REAL = ( -76.71373, 39.25490)


IMAGE_NAME = "road.png"
#you need to already know the real location of the pixel, the conversion will be based on this so make it as accurate as possible
TOP_RIGHT_PIX = (1149,200)
TOP_RIGHT_REAL = (-76.70702,39.26021)

BOTTOM_LEFT_PIX = (630, 824)
BOTTOM_LEFT_REAL = ( -76.71596, 39.25190)

IMAGE_NAME = "lower_area.png"
#you need to already know the real location of the pixel, the conversion will be based on this so make it as accurate as possible
TOP_RIGHT_PIX = (1397,59)
TOP_RIGHT_REAL = (-76.70990,39.25399)

BOTTOM_LEFT_PIX = (20, 732)
BOTTOM_LEFT_REAL = ( -76.71581, 39.25176)

"""
IMAGE_NAME = "most_main_campus.png"
#you need to already know the real location of the pixel, the conversion will be based on this so make it as accurate as possible
TOP_RIGHT_PIX = (1723,92)
TOP_RIGHT_REAL = (-76.70919,39.25589)

BOTTOM_LEFT_PIX = (443, 764)
BOTTOM_LEFT_REAL = ( -76.71473, 39.25365)


# gets the bounds of the real word coords
def get_bounds(pixel_pair, corrd_pair, width, height):
    pixel_pair[0] = (pixel_pair[0][0],height - pixel_pair[0][1])
    pixel_pair[1] = (pixel_pair[1][0],height - pixel_pair[1][1])
    w_e_x= (corrd_pair[1][0] - corrd_pair[0][0])/((pixel_pair[1][0] - pixel_pair[0][0])/width)
    w_e_y= (corrd_pair[1][1] - corrd_pair[0][1])/((pixel_pair[1][1] - pixel_pair[0][1])/height)
    left_bound = corrd_pair[0][0] - pixel_pair[0][0]*(w_e_x/width)
    right_bound = left_bound + w_e_x
    bottom_bound =corrd_pair[0][1] - pixel_pair[0][1]*(w_e_y/height)
    top_bound = bottom_bound + w_e_y
    return [(left_bound, bottom_bound), (right_bound, top_bound)]

#gets the real world cord from a pixel
def get_real_cord(pixel_cord,width,height,bounds):
    pixel_cord = (pixel_cord[0],height-pixel_cord[1])
    percent_x = pixel_cord[0]/width
    percent_y = pixel_cord[1]/height
    bound_width = bounds[1][0] - bounds[0][0]
    bound_height = bounds[1][1] - bounds[0][1]
    x_cord_out = bounds[0][0] + percent_x * bound_width
    y_cord_out = bounds[0][1] + percent_y * bound_height
    return (x_cord_out,y_cord_out)


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
                            is_blue = b == 255 and g == 0 and r ==0
                            is_red = r == 255  and b == 0 and g ==0
                            is_green = g == 255 and r == 0 and b ==0
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
        bounds = get_bounds([TOP_RIGHT_PIX, BOTTOM_LEFT_PIX], [TOP_RIGHT_REAL, BOTTOM_LEFT_REAL], width, height)

        # Iterate over all pixels
        for i in range(width):
            for j in range(height):
                # Get the RGB tuple of the current pixel
                r, g, b = pixels[i, j]
                #check to make sure white is ingnored
                #if g < color_threshold and b < color_threshold:
                     # Check if the pixel is predominantly red
                     #(r == 255 and g == 0 and b == 0) or (g == 255 and r == 0 and b ==0):
                if (r == 255 and g == 0 and b == 0):
                    curr_cord = (i , j)
                    #array that stores the coords for each mpo
                    mpo_list.append(curr_cord)
                    #find_blue(pixels, color_threshold, width, height, mpo_cords, curr_cord)
        print(len(mpo_list)) #for testing
        print("found nodes")
        draw = ImageDraw.Draw(img)
        #file = open("Nodes8.txt", "w")
        for i in range(len(mpo_list)):
            number_to_write = i + 737;
            x = mpo_list[i][0] + 1
            y = mpo_list[i][1]
            draw.text((x, y), str(number_to_write), fill = (255,150,0))
            real_coord = get_real_cord(mpo_list[i], width, height, bounds)
            round_x = round(real_coord[0], 5)
            round_y = round(real_coord[1], 5)
            mpo_list[i] = (round_x, round_y)
            spacing = "    "
            text = str(i+737) + spacing + str(mpo_list[i][0]) + "    " + str(mpo_list[i][1]) + "\n"
            print(text)
            #file.write(text)
        #file.close

        #used dislay the green and red pixels turned orange, only for testing to see if it worked properly
        img.save("temp2", format="png")
        img.show()

    except FileNotFoundError:
        print(f"Error: The file at {image_path} was not found.")
    except Exception as e:
        print(f"An error occurred: {e}")

