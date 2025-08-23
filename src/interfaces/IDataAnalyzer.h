#ifndef IDATAANALYZER_H
#define IDATAANALYZER_H

class IDataAnalyzer {
public:
    virtual ~IDataAnalyzer() = default;
    virtual void analyzeData() = 0;
};

#endif // IDATAANALYZER_H
