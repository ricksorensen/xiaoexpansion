import time


def setPCF(pcf, gpstime):
    pcf.set(
        gpstime[3], gpstime[4], gpstime[5], 0, gpstime[2], gpstime[1], gpstime[0] % 100
    )


# samd takes 7 - tuple
# rp2  takes 8 - tuple
def setRTC(rtc, gpstime):
    if rtc is not None:
        rtc.datetime(
            (
                gpstime[0],
                gpstime[1],
                gpstime[2],
                0,
                gpstime[3],
                gpstime[4],
                gpstime[5],
                0,
            )
        )


class AllTime:
    def __init__(self, gpsUart=None, pcf=None, rtc=None):
        self.gpsUart = gpsUart
        self.pcf = pcf
        self.rtc = rtc
        self.t0 = self.setTimers(waittime=20)

    def setTimers(self, waittime=8):
        gt = self.getGPSTime(waittime=waittime)
        if gt:
            setPCF(self.pcf, gt)
        else:
            pt = self.pcf.get()
            if pt[6] > 20 and pt[6] < 30:
                gt = [pt[6] + 2000, pt[5], pt[4], pt[0], pt[1], pt[2]]
            else:
                gt = [2023, 2, 28, 12, 13, 14]
                setPCF(self.pcf, gt)
                gt[3] = gt[3] + 1  # bump an hour
        if self.rtc is not None:
            setRTC(self.rtc, gt)
        return gt

    def getGPSTime(self, waittime=8):
        timeout = time.time() + waittime
        tryagain = True
        if self.gpsUart is not None:
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
                            print("GPS ", GPSdate)
                            return GPSdate
                        except ValueError:
                            print("Bad RMC line")
                            print(parts)
                tryagain = time.time() <= timeout
                time.sleep_ms(100)
                print(".", end="")
        print("No GPS Time Found")
        return None
