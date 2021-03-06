"""
Examples to run networks with batched inputs on Micron Deep Learning Accelerator
"""

import cv2
import numpy as np

from argparse import ArgumentParser

# Clear screen
#print('\033[0;0f\033[0J')
# Color Palette
CP_R = '\033[31m'
CP_G = '\033[32m'
CP_Y = '\033[33m'
CP_C = '\033[36m'
CP_0 = '\033[0m'

# List of models which can be run by this example script
models = ['yolov3', 'yolov3_tiny',  'inception', 'resnet34_50','resnet34_18','superresolution']
model_names = sorted(name for name in models)

parser = ArgumentParser(description="Micron DLA Examples")
_ = parser.add_argument
_('--model', type=str, default='linknet', help='Model architecture:' + ' | '.join(model_names) + ' (default: linknet)')
_('--bitfile', type=str, default='', help='Path to the bitfile')
_('--model-path', type=str, default='', help='Path to the NN model')
_('--numfpga', type=int, default=1, help='Number of FPGAs to use')
_('--numclus', type=int, default=1, help='Number of clusters to use')
args = parser.parse_args()


def main():
    print('{:=<80}'.format(''))
    print('{}Micron{} DLA Examples{}'.format(CP_C, CP_Y, CP_0))
    print('{:-<80}'.format(''))

    if args.model == 'superresolution':
        from SuperResolution.superresolution import SuperResolutionDLA
        input_array = np.random.rand(args.numclus * args.numfpga, 1, 224, 224)
        superresolution = SuperResolutionDLA(input_array, args.bitfile, args.model_path, args.numfpga, args.numclus)
        model_output = superresolution(input_array)
        del superresolution

    elif args.model == 'yolov3':
        from YOLOv3.yolov3 import YOLOv3
        input_array = np.random.rand(args.numfpga, 416, 416, 3)
        yolov3 = YOLOv3(input_array, args.bitfile, args.model_path, args.numfpga, False)
        model_output = yolov3(input_array)
        del yolov3

    elif args.model == 'yolov3_tiny':
        from YOLOv3.yolov3 import YOLOv3Tiny
        input_array = np.random.rand(args.numfpga, 416, 416, 3)
        yolov3 = YOLOv3Tiny(input_array, args.bitfile, args.model_path, args.numfpga, False)
        model_output = yolov3(input_array)
        del yolov3

    elif args.model == 'inception':
        #Example Mode 1
        # This is an example of one model (inception_v3) applied to 2 images
        # on 1 fpga and 2 clusters
        from Inception.inception import InceptionDLA
        input_array = np.random.rand(16, 299, 299, 3)
        inception = InceptionDLA(input_array, args.bitfile, args.model_path, args.numfpga, args.numclus)
        model_output = inception(input_array)
        del inception

    elif args.model == 'resnet34_50':
        #Example for Mode 4:
        # This is an example of two models (resnet34 and resnet50) applied to 2 images
        # on 2 fpga and 1 clusters
        from Resnet.resnet34_50 import Resnet34_50DLA
        input_array = np.random.rand(2, 224, 224, 3)
        model_path=args.model_path.split(',')
        resnet = Resnet34_50DLA(input_array, args.bitfile, model_path[0], model_path[1], args.numfpga, args.numclus)
        model_output1, model_output2 = resnet(input_array[0], input_array[1])
        # The model was applied on 2 images; the resnet returns - one output for each image
        del resnet

    elif args.model == 'resnet34_18':
        #Example for Mode 3:
        #This is an example of two models (resnet34 and resnet18) applied to 2 images
        # on 1 fpga and 1 clusters
        from Resnet.resnet34_18 import Resnet34_18DLA
        input_array = np.random.rand(2, 224, 224, 3)
        model_path=args.model_path.split(',')
        resnet = Resnet34_18DLA(input_array, args.bitfile, model_path[0], model_path[1], args.numfpga, args.numclus)
        model_output1, model_output2 = resnet(input_array[0], input_array[1])
        # The model was applied on 2 images; the resnet returns - one output for each image
        del resnet

    else:
        print('{}Invalid model selection{}!!!'.format(CP_R, CP_0))

    print('{}Example run successful{}.'.format(CP_G, CP_0))
    print('{:=<80}\n'.format(''))


if __name__ == "__main__":
    main()
