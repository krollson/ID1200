import matplotlib.pyplot as plt
import os

directory = "/Users/kristoffer/KTH/ID1200-os/lab5/"

def scatter_plot_with_line(x, y, name):
    if len(x) != len(y):
        raise ValueError(f"The two lists must have the same length - y: {len(y)} x : {len(x)}")
    
    COLOR = 'deepskyblue'

    plt.figure(figsize=(8, 6))
    ax = plt.gca()  # Get the current axis

    # Plot the scatter points with the same color
    plt.scatter(y, x, color=COLOR, label="Data Points")
    
    # Plot the line segments and add arrows
    for i in range(len(y) - 1):  # Loop through each consecutive pair of points
        # Draw line segment
        plt.plot([y[i], y[i+1]], [x[i], x[i+1]], linestyle='-', marker='o', color=COLOR)
        
        # Add arrow for direction
        plt.annotate(
            '',  # No text
            xy=(y[i+1], x[i+1]),  # End point of the arrow
            xytext=(y[i], x[i]),  # Start point of the arrow
            arrowprops=dict(
                arrowstyle='->', 
                color=COLOR,  # Match the line and points color
                lw=1
            )
        )

    plt.title(f"Movement illustration for algorithm {name}", pad=20)

    # Move the x-axis to the top
    ax.xaxis.set_label_position('top')
    ax.xaxis.tick_top()
    plt.xlabel(f"Cylinder Rotations for Algorithm {name}", labelpad=10)

    # Remove y-axis labels and ticks
    ax.yaxis.set_ticks([])
    ax.yaxis.set_ticklabels([])

    # Set x-axis limits to match the data range
    plt.xlim(0, 3000)
    te = 0.2
    plt.ylim(min(x) - te, max(x) + te)

    # Flip the graph upside down
    plt.gca().invert_yaxis()

    file_path = os.path.join(directory, f"{name}.png")
    plt.savefig(file_path)
    plt.show()
    plt.close()
    print(f"Results and plots saved in {directory} as {file_path}")
    write_rotations_to_file(y, name)
    
def get_total_number_of_rotations(y):
    sum = 0
    for i in range(len(y) - 1):
        diff = abs(y[i + 1] - y[i])
        sum = sum + diff
        print(f"Sum is {sum}")
        print(f"Diff is {diff}")
        with open(txt_directory, "a") as file:
            file.write(f"Diff: {diff}\n")
    return sum

def write_rotations_to_file(y, name):
    value = get_total_number_of_rotations(y)
    with open(txt_directory, "a") as file:
        print(f"Value {name} - {value} written to {txt_directory}")
        file.write(f"{name} - {value}\n")
        file.write("\n")
        

x = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
standard = [1600, 1800, 1912, 260, 420, 280, 2024, 2788, 2901, 1400, 1600, 1501]
x2 = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]

y_vals_1 = [1800, 1912, 2024, 1600, 1501, 1400, 420, 280, 260, 2788, 2901]
y_vals_2 = [1800, 1912, 2024, 2788, 2901, 2999, 1600, 1501, 1400, 420, 280, 260]
y_vals_3 = [1800, 1912, 2024, 2788, 2901, 260, 280, 420, 1400, 1501, 1600]
txt_directory = os.path.join(directory, "values.txt")

with open(txt_directory, "w") as file:
    pass
print(f"File {txt_directory} successfully cleared")
scatter_plot_with_line(x, y_vals_1, "SSTF")
scatter_plot_with_line(x2, y_vals_2, "SCAN")
scatter_plot_with_line(x, y_vals_3, "C-SCAN")