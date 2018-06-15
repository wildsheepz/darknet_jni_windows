#include <assert.h>
#include <fstream>
#include <iostream>
#include <cmath>

#include "wildsheep_darknet_DarknetUtils.h"
#include "DarknetAPI.h"

static DarknetAPI * api ;
image readRawImageData(std::string image_path);
/*
* Class:     wildsheep_darknet_DarknetUtils
* Method:    inference
* Signature: (Ljava/lang/String;)[Lwildsheep/darknet/Result;
*/
JNIEXPORT jobjectArray JNICALL Java_wildsheep_darknet_DarknetUtils_inference
(JNIEnv * env, jobject, jstring image_name_j) {
	std::cout << "JNI: Inference starting"<< std::endl;

	std::string image_name  = env->GetStringUTFChars(image_name_j, (jboolean)false);
	image im = readRawImageData(image_name);
	//image im2 = copy_image(im);
	std::vector<Result> results = api->detect(im);
	std::cout << "JNI: Detection results count: " << results.size() << std::endl;
	
	jclass c = (*env).FindClass("wildsheep/darknet/Result");
	jmethodID mid = NULL;
	jsize size = results.size();


	jobjectArray returnArr = (*env).NewObjectArray(size, c, NULL);
	if (NULL != c) {
		mid = (*env).GetMethodID(c, "<init>", "(IIIIFLjava/lang/String;)V");
		if (NULL != mid) {
			for (int i = 0; i < results.size(); ++i) {
				Result r = results[i];
				jint l = r.left;
				jint jr = r.right;
				jint t = r.top;
				jint b = r.bot;
				jstring jlabel = env->NewStringUTF(r.label.c_str());
				jfloat conf = r.confidence;

				jobject jObj = (*env).NewObject(c, mid, l, t, jr, b, conf, jlabel);
				(*env).SetObjectArrayElement(returnArr, i, jObj);
				std::cout << "JNI: " << r.label << " " << r.left << " " << r.top << " " << r.right << " " << r.bot << " " << r.confidence << std::endl;
				//draw_box(im2, r.left, r.top, r.right, r.bot, 255, 0, 0);
			}
		}
	}
	//save_image_png(im2, "test");
	//free_image(im2);
	std::cout << "JNI: Inference routine complete" << std::endl;
	return returnArr;
}

int readIntFromByteArr(char * buf, int offset, int len) {
	int sum = 0;
	
	for (int i = 0, shift = 0; i < len; ++i, shift+=8) {
		sum += int((unsigned char )buf[offset + i] << shift);
	}
	return sum;
}

const static int PixelFormat_AndroidAPI19_RGB_565 = 4;
const static int PixelFormat_AndroidAPI19_TRANSLUCENT = -3;
const static int PixelFormat_AndroidAPI19_RGBA_8888 = 1;
const static int PixelFormat_AndroidAPI19_RGBX_8888 = 2;
const static int PixelFormat_AndroidAPI19_RGB_888 = 3;

unsigned int *  bandmask(int pixelFormat) {
	switch (pixelFormat) {
	case PixelFormat_AndroidAPI19_RGBA_8888:
		return new unsigned int[4] { 0xFF000000, 0xFF0000, 0xFF00, 0xFF };
	case PixelFormat_AndroidAPI19_RGB_888:
		return new unsigned int[3] { 0xFF0000, 0xFF00, 0xFF };
	case PixelFormat_AndroidAPI19_RGB_565:
		return new unsigned int[3] { 0xF80000, 0x7E0, 0x1F };
	}
}

unsigned int * bitshift(int pixelFormat) {
	switch (pixelFormat) {
	default:
	case PixelFormat_AndroidAPI19_RGBA_8888:
		return new unsigned int[4] { 24, 16, 8, 0 };
	case PixelFormat_AndroidAPI19_RGB_888:
		return new unsigned int[3] { 16, 8, 0 };
	case PixelFormat_AndroidAPI19_RGB_565:
		return new unsigned int[3] { 19, 5, 0 };
	}
}


image readRawImageData(std::string image_path) {
	std::ifstream infile;
	char buf[4];
	infile.open(image_path, std::ios::binary);
	//infile.seekg(std::ifstream::end);
	//const size_t filesize = infile.tellg();
	//const size_t pixel_data_size = filesize - 12;
	infile.seekg(std::ifstream::beg);


	infile.read(buf,4);
	int width = readIntFromByteArr(buf, 0, 4);
	infile.read(buf, 4);
	int height= readIntFromByteArr(buf, 0, 4);
	infile.read(buf, 4);
	int pixelFormat = readIntFromByteArr(buf, 0, 4);
	
	image im = make_image(width, height, 3);

	std::cout << "JNI: " << im.c << "x" << im.w << "x" << im.h << std::endl;

	char * pixel;
	int pixel_size = -1;
	switch (pixelFormat) {
	default:
	case PixelFormat_AndroidAPI19_RGB_888:
		pixel = new char[3];
		pixel_size = 3;
		break;
	case PixelFormat_AndroidAPI19_RGBA_8888:
		pixel = new char[4];
		pixel_size = 4;
		break;
	case PixelFormat_AndroidAPI19_RGB_565:
		pixel = new char[2];
		pixel_size = 2;
		break;
	}


	unsigned int * bandmasks = bandmask(pixelFormat);
	unsigned int * bitshifts = bitshift(pixelFormat);
	int pixel_idx = 0;
	
	std::cout << "JNI: " << "pixelFormat: " << pixelFormat << std::endl;
	std::cout << "JNI: " << "bytes per pixel: " << pixel_size << std::endl;


	while (infile.peek() != EOF){
		infile.read(pixel, pixel_size);
		int value = readIntFromByteArr(pixel, 0, pixel_size);
		int r = (bandmasks[3] & value) >> bitshifts[3];
		int g =( bandmasks[2] & value) >> bitshifts[2];
		int b = (bandmasks[1] & value) >> bitshifts[1];
		int a = (bandmasks[0] & value) >> bitshifts[0];
		int x = pixel_idx % width;
		int y = (pixel_idx - x) / width;

		//std::cout << x << " " << y << std::endl;
		//break;
		//std::cout << r << " " << g << " " << b  << " " << a << std::endl;
		set_pixel(im, x, y, 0, ((float)r)/255 );
		set_pixel(im, x, y, 1, ((float)g)/255 );
		set_pixel(im, x, y, 2, ((float)b)/255 );
		++pixel_idx;
	}
	//std::cout << "JNI: " << get_pixel(im, 10, 10, 2) << std::endl;
	infile.close();

	//print_image(im);

	delete[] (bandmasks);
	delete[] (bitshifts);
	delete[] (buf);
	delete[](pixel);
	return im;
}

/*
* Class:     wildsheep_darknet_DarknetUtils
* Method:    load
* Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z
*/
JNIEXPORT jboolean JNICALL Java_wildsheep_darknet_DarknetUtils_load
(JNIEnv * env, jobject obj, jstring datacfg_j, jstring name_list_file_j, jstring cfgfile_j, jstring weightfile_j) {
	std::cout << "JNI: " << "Loading network and weights.." << std::endl;

	std::string datacfg = env->GetStringUTFChars(datacfg_j, (jboolean)false);
	std::string name_list_file = env->GetStringUTFChars(name_list_file_j, (jboolean)false);
	std::string cfgfile = env->GetStringUTFChars(cfgfile_j, (jboolean)false);
	std::string weightfile = env->GetStringUTFChars(weightfile_j, (jboolean)false);
	
	api = new DarknetAPI(&datacfg[0u], &name_list_file[0u], &cfgfile[0u], &weightfile[0u]);
	assert(api && "Failed to initialize DarknetAPI");
	jboolean b = 1;
	std::cout << "JNI: " << "Done." << std::endl;
	return b;
}

/*
* Class:     wildsheep_darknet_DarknetUtils
* Method:    unload
* Signature: ()Z
*/
JNIEXPORT jboolean JNICALL Java_wildsheep_darknet_DarknetUtils_unload
(JNIEnv *, jobject) {
	delete(api);
	jboolean b = 1;
	std::cout << "JNI: " << "Unloaded network from memory." << std::endl;
	return b;
}