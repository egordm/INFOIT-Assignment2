//
// Created by egordm on 19-3-2018.
//

#include "window_creators.h"

ComplexDatasetData load_complex_dataset_data(const file::Dataset *datasets, int dataset_count, const std::string &save_name) {
    std::stringstream ss;
    ss << SAVES_PATH << save_name << ".save";

    ComplexDatasetData ret;

    if(cvision::processing::file::file_exists(ss.str())) {
        std::cout << "Reading " << ss.str() << std::endl;
        FileStorage fs;
        fs.open(ss.str(), FileStorage::READ);
        M_Assert(fs.isOpened(), "Could not open the save file");

        ret.marginal_positive_prob = (float) fs["marginal_positive_prob"];

        ret.env_hst = new image::Histogram3D();
        ret.env_hst->read(fs["env_hst"]);

        ret.positive_hst = new image::Histogram3D();
        ret.positive_hst->read(fs["positive_hst"]);
    } else {
        unsigned long total_images = 0;
        ret.positive_hst = nullptr;
        ret.env_hst = nullptr;
        for(int i = 0; i < dataset_count; ++i) {
            auto dataset_images = file::load_dataset(datasets[i]);

            Mat dataset_inputs[dataset_images.size()];
            Mat dataset_masks[dataset_images.size()];
            Mat dataset_masks_inv[dataset_images.size()];

            for (int i = 0; i < dataset_images.size(); ++i) {
                dataset_inputs[i] = dataset_images[i].input;
                dataset_masks[i] = dataset_images[i].label;
                bitwise_not(dataset_masks[i], dataset_masks_inv[i]);
            }

            const float ranges[3] = {256, 256, 256};

            auto positive_hst = image::extract_histogram_3d(dataset_inputs, (uint) dataset_images.size(), ranges, dataset_masks, 128);
            positive_hst->histogram *= dataset_images.size();
            auto env_hst = image::extract_histogram_3d(dataset_inputs, (uint) dataset_images.size(), ranges, dataset_masks_inv,128);
            env_hst->histogram *= dataset_images.size();
            auto marginal_positive_prob = image::probability_masked_pixels(dataset_masks, (uint) dataset_images.size());

            if(ret.positive_hst == nullptr) ret.positive_hst = positive_hst;
            else ret.positive_hst->histogram += positive_hst->histogram *dataset_images.size();

            if(ret.env_hst == nullptr) ret.env_hst = env_hst;
            else ret.env_hst->histogram += env_hst->histogram * dataset_images.size();

            ret.marginal_positive_prob += marginal_positive_prob * dataset_images.size();
            total_images += dataset_images.size();
        }
        ret.positive_hst->normalize();
        //ret.positive_hst->histogram /= total_images;
        ret.env_hst->normalize();
        //ret.env_hst->histogram /= total_images;
        ret.marginal_positive_prob = ret.marginal_positive_prob / total_images;

        {
            std::cout << "Writing " << ss.str() << std::endl;
            FileStorage fs(ss.str(), FileStorage::WRITE);
            fs << "marginal_positive_prob" << ret.marginal_positive_prob;
            fs << "env_hst";
            ret.env_hst->write(fs);
            fs << "positive_hst";
            ret.positive_hst->write(fs);

            fs.release();
        }
    }

    return ret;
}