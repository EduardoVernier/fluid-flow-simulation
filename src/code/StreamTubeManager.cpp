#include "StreamTubeManager.h"

void StreamTubeManager::add_stream_tube(int mx, int my)
{
    StreamTube st = StreamTube(mx, my);
    stream_tube_vector.push_back(st);
}

void StreamTubeManager::remove_stream_tube(int mx, int my)
{

}
