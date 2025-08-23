#ifndef IHEATMAPGENERATOR_H
#define IHEATMAPGENERATOR_H

class IHeatMapGenerator {
public:
    virtual ~IHeatMapGenerator() = default;
    virtual void generateHeatMap() = 0;
};

#endif // IHEATMAPGENERATOR_H
