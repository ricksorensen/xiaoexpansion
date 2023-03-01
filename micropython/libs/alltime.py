import time


def setPCF(pcf, gpstime):
    pcf.set(
        gpstime[3], gpstime[4], gpstime[5], 0, gpstime[2], gpstime[1], gpstime[0] % 100
    )


# samd takes 7 - tuple
# rp2  takes 8 - tuple
def setRTC(rtc, gpstime):
    rtc.datetime(
        (gpstime[0], gpstime[1], gpstime[2], 0, gpstime[3], gpstime[4], gpstime[5], 0)
    )


class AllTime:
    def __init__(self, gpsUart=None, pcf=None, rtc=None):
        self.gpsUart = gpsUart
        self.pcf = pcf
        self.rtc = rtc

        self.t0 = self.getGPSTime(waittime=20)
        if self.t0:
            setPCF(self.pcf, self.t0)
            setRTC(self.rtc, self.t0)

    def getGPSTime(self, waittime=8):
        timeout = time.time() + waittime
        tryagain = True
        while self.gpsUart.any() > 0:
            self.gpsUart.read()
        while tryagain:
            parts = str(self.gpsUart.readline()).split(",")
            # print(parts)
            if ("RMC" in parts[0]) and (len(parts) >= 10):
                if parts[1] and parts[9]:
                    try:
                        # day-mon-yr
                        GPSdate = [
                            int(parts[9][4:6]) + 2000,
                            int(parts[9][2:4]),
                            int(parts[9][0:2]),
                        ]
                        # hr:min:sec (GMT)
                        GPStime = [
                            int(parts[1][0:2]),
                            int(parts[1][2:4]),
                            int(parts[1][4:6]),
                        ]
                        GPSdate.extend(GPStime)
                        # print(GPSdate)
                        return GPSdate
                    except ValueError:
                        print("Bad RMC line")
                        print(parts)
            tryagain = time.time() <= timeout
            time.sleep_ms(100)
        return None
