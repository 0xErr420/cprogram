#ifndef ANALYZER_H
#define ANALYZER_H

/// Analyzer thread reads 'cpu' lines from buffer, analyzes,
/// calculates percentages and sends them to specified buffer
void *thread_Analyzer(void *arg);

#endif