#ifndef ANALYZER_H
#define ANALYZER_H

/// Analyzer thread: analyzes fields of each cpu, calculates percentages and sends them to specified buffer
///
/// ARG fields required: Consumer, Producer
void *thread_Analyzer(void *arg);

#endif