#ifndef __OTA_H__
#define __OTA_H__

void onOTAStart();
void onOTAProgress(size_t current, size_t final);
void onOTAEnd(bool success);

#endif // __OTA_H__
