# import pandas as pd

# class DataObject:
#     def __init__(self, id, ra, ll, la, v1, x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4):
#         self.id = id
#         self.ra = ra
#         self.ll = ll
#         self.la = la
#         self.v1 = v1
#         self.x1 = x1
#         self.y1 = y1
#         self.z1 = z1
#         self.x2 = x2
#         self.y2 = y2
#         self.z2 = z2
#         self.x3 = x3
#         self.y3 = y3
#         self.z3 = z3
#         self.x4 = x4
#         self.y4 = y4
#         self.z4 = z4

# def dummy_data():
#     df = pd.read_csv("Data_4.csv")
#     reading = []
#     for i in range(len(df)):
#         row = df.iloc[i]
#         id = i
#         for j in range(len(row)):
#             ra = row[0]
#             ll = row[1]
#             la = row[2]
#             v1 = row[3]
#             if i%10 == 0:
#                 x1 = row[4]
#                 y1 = row[5]
#                 z1 = row[6]
#                 x2 = row[7]
#                 y2 = row[7]
#                 z2 = row[9]
#                 x3 = row[10]
#                 y3 = row[11]
#                 z3 = row[12]
#                 x4 = row[13]
#                 y4 = row[14]
#                 z4 = row[15]
#             else:
#                 x1 = y1 = z1 = x2 = y2 = z2 = x3 = y3 = z3 = x4 = y4 = z4 = -1
#         print('\n', id, ' ', id, ' ', ra, ' ', ll, ' ', la, ' ', v1, ' ', x1, ' ', y1, ' ', z1, ' ', x2, ' ', y2, ' ', z2, ' ', x3, ' ', y3, ' ', z3, ' ', x4, ' ', y4, ' ', z4, '\n')
#     return DataObject(id, ra, ll, la, v1, x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4)

# while True:
#     data = dummy_data()

import random

class ACC:

    def __init__(self, qcount=0, batch=0):
        self.BHI = {i: self.Sensor(i, 0, 0, 0) for i in range(1, 5)}
        self.sample = [self.BHI[_].sample_1 for _ in range(1, 5)]
        self.qcount = qcount
        self.batch = batch

    class Sensor:
        
        def __init__(self, imu_num=0, x=0, y=0, z=0):
            self.imu_num = imu_num
            self.x = x
            self.y = y
            self.z = z
            self.sample_1 = [x, y, z]

        def update_sample(self):
            self.x = random.randint(-100, 100)
            self.y = random.randint(-100, 100)
            self.z = random.randint(-100, 100)
            self.sample_1 = [self.x, self.y, self.z]

    def generate_random_data(self):
        for sensor in self.BHI.values():
            sensor.update_sample()
        self.sample = [self.BHI[_].sample_1 for _ in range(1, 5)]

acc = ACC()
print(acc.BHI[1].sample_1)  # Output: [0, 0, 0]
print(acc.sample)  # Output: [[0, 0, 0], [0, 0, 0], [0, 0, 0], [0, 0, 0]]

acc.generate_random_data()
print(acc.BHI[1].sample_1)  # Output: [random value, random value, random value]
print(acc.sample)  # Output: [[random value, random value, random value], [random value, random value, random value], [random value, random value, random value], [random value, random value, random value]]
