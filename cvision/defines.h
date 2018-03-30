//
// Created by egordm on 14-3-2018.
//

#ifndef B3ITASSIGNMENT2_DEFINES_H
#define B3ITASSIGNMENT2_DEFINES_H

#define DEBUG true

#ifdef _WIN32
#define FILE_SEPARATOR "\\"
#else
#define FILE_SEPARATOR "/"
#endif

#define DATASETS_PATH (".." FILE_SEPARATOR ".." FILE_SEPARATOR "data" FILE_SEPARATOR "datasets" FILE_SEPARATOR)
#define OPENCV_DATASET_PATH (".." FILE_SEPARATOR ".." FILE_SEPARATOR "data" FILE_SEPARATOR "opencv_data" FILE_SEPARATOR)
#define SAVES_PATH (".." FILE_SEPARATOR ".." FILE_SEPARATOR "data" FILE_SEPARATOR "saves" FILE_SEPARATOR)

#define DATASET_INPUTS ("inputs" FILE_SEPARATOR)
#define DATASET_LABELS ("labels" FILE_SEPARATOR)

#define EPSILON 0.001

#endif //B3ITASSIGNMENT2_DEFINES_H
