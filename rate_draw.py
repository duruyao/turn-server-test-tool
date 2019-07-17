import os, sys, getopt
import math, matplotlib
import numpy as np
import matplotlib.pyplot as plt

N = 0
IMG_DIR, data_file_dir = ['rate_img.png', '']
rate_array = []
rate_max, rate_ave = [0., 0.]

## Program'll adjust the following parameters to change the picture layout.
show_img = False
img_width, img_height = [16, 9]
v_gap, h_gap = [5, 5]
v_max, h_max = [N, N]

def usage():
    """
    Usage:  python3 rate_draw.py [Flags] [Options]
    Flags:
                -h <help> Show the usage of rate_draw.py.
                -s <show> If show the image.
    Options:    
                -p <path> The direction of data file.
    Example:    
                python3 rate_draw.py -p 19-07-16/data_2_py.txt
    """


def load_data():
    global N, IMG_DIR, data_file_dir, rate_array, rate_max, rate_ave
    global show_img, img_width, img_height, v_gap, h_gap, v_max, h_max
    for line in open(data_file_dir, mode='r'):
        if not len(line):
            continue
        N = N + 1
        data = float(line)
        rate_ave += data
        rate_max = max(rate_max, data)
        rate_array.append(data)
    if N:
        rate_ave = rate_ave / N
        v_max, h_max = [rate_max, N]
        v_gap = (int((100 * rate_max) / 20) + 1) / 100
        h_gap = max(1, math.ceil(N / 20))
    if rate_ave <= 0.0001:
        print("[*] average rate is zero, it's not necessary to draw it.")
        sys.exit()


def draw_img():
    global N, IMG_DIR, data_file_dir, rate_array, rate_max, rate_ave
    global show_img, img_width, img_height, v_gap, h_gap, v_max, h_max
    x = np.linspace(1, N, N)
    y = np.asarray(rate_array)

    ## Set size of image.
    plt.figure(figsize=(16, 9))
    
    ## Set coordinate.
    plt.xlim((1, N))
    plt.ylim((0, v_max))
    plt.xlabel('number of times')
    plt.ylabel('packet loss rate (%)')
    x_ticks = np.arange(0, h_max + h_gap, h_gap)
    y_ticks = np.arange(0 - v_gap, v_max + v_gap, v_gap)
    plt.xticks(x_ticks)
    plt.yticks(y_ticks)
    
    ## Set grid.
    plt.grid(axis='y', linestyle='--')
    
    ## Draw scatter point.
    plt.scatter(x, y, s=10, color='blue', alpha=1.0)
    
    ## Set title.
    plt.title('Packet Loss Rate Test - Multiple Rds')

    ## Draw average rate.
    plt.scatter(x, y, s=10, color='blue', alpha=1.0)
    plt.plot([0, 100], [rate_ave, rate_ave], color='green', label='Average Rate: ' + str(rate_ave) + '%')
    plt.legend()

    ## save image.
    plt.savefig(IMG_DIR)
    print('[*] draw and save image to ' + IMG_DIR)
    if show_img:
        plt.show()
    
    
def main():
    global N, IMG_DIR, data_file_dir, rate_array, rate_max, rate_ave
    global show_img, img_width, img_height, v_gap, h_gap, v_max, h_max
    
    ## Handle options and  arguments.
    opts, args = ['', '']
    if not len(sys.argv[1:]):
        print(usage.__doc__)
        sys.exit()
    try:
        opts, args = getopt.getopt(sys.argv[1:], "hsp:", ["help", "show", "path"])
    except getopt.GetoptError as e:
        print(e)
        print(usage.__doc__)
        sys.exit()
    for opt_key, opt_value in opts:
        if opt_key in ('-h', '--help'):
            print(usage.__doc__)
            sys.exit()
        elif opt_key in ('-s', '--show'):
            show_img = True
        elif opt_key in ('-p', '--path'):
            data_file_dir = opt_value
            if not (os.path.exists(data_file_dir)):
                print('[*] no such file.')
                sys.exit()
    
    ## Read data of packet of lossing rate from file.
    load_data()
    
    ## draw and save image.
    draw_img()


if __name__ == '__main__':
    main()
    
