#ifndef __DIRECTSHOw_RESOLUTION__
#define __DIRECTSHOw_RESOLUTION__

#include <vector>
#include <map>
#include <iostream>
#include <algorithm>

namespace cam_resolutioin {
	typedef std::vector<std::pair<int, int>> vec_camera_resolution_;
	typedef std::map<std::string, vec_camera_resolution_> map_camera_detail_;

	#define BLUE    0x0001
	#define GREEN   0x0002
	#define RED     0x0004
	#define GRAY    0x0007

	void setcolor(unsigned int color);
	map_camera_detail_ enum_devices();

}	//cam_resolutioin

#endif	//__DIRECTSHOw_RESOLUTION__