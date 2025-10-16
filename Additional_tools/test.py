from PIL import Image

if __name__ == "__main__":
    image_path = 'google.png'
    green_threshold = 150
    try:
        # Open the image file
        img = Image.open(image_path).convert('RGB')
        width, height = img.size
        #gets the pixles from the image
        pixels = img.load()
        #array of tuples that scores the location of the green pixles
        green_pixles =[]
        # Iterate over all pixels
        for i in range(width):
            for j in range(height):
                # Get the RGB tuple of the current pixel
                r, g, b = pixels[i, j]
                # Check if the pixel is predominantly green
                if g > green_threshold and g > r and g > b:
                    green_pixles.append((i,j))
                    #pixels[i, j] = (0, 0, 0) #used to change green pixles to black, only used to check if the green pixels were all found

        for i in range(len(green_pixles)):
                print(green_pixles[i])
        #used dislay the green pixels turned black, only for testing to see if ti worked properly
        #img.save("black", format="png")
        #img.show()

    except FileNotFoundError:
        print(f"Error: The file at {image_path} was not found.")
    except Exception as e:
        print(f"An error occurred: {e}")
