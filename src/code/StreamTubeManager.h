#ifndef STREAMTUBEMANAGER_H
#define STREAMTUBEMANAGER_H

#include <vector>
#include "StreamTube.h"
using namespace std;

class StreamTubeManager
{
public:
    StreamTubeManager() {stream_tube_vector.clear();}
    void add_stream_tube(int mx, int my);
    void remove_stream_tube(int mx, int my);

    vector <StreamTube> stream_tube_vector;
};

#endif
