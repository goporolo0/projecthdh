**PROJECT 1:**
Bước 1 : Cài đặt Linux trên máy ảo

Bước 2 : Tải và giải nén file projecthdh.rar

Bước 3 : Vào thư mục projecthdh/gnu-decstation-ultrix/decstation-ultrix/2.95.3/  Thiết lập thuộc tính excutable ở mọi người dùng đối với tất cả các file có trong thư mục (as, cc1, cpp0, gcc, ld).

Bước 4 : Sửa dòng “MAKE = gmake” thành MAKE = make và lưu lại.

Bước 5 : cd vào /code 

Bước 6 : Tiếp tục gõ make để tiến hành cài đặt NachOS 

Bước 7 : Chạy thử chương trình trên NachOS bằng lệnh  ./userprog/nachos –rs 1023 –x ./test/halt 

**PROJECT 2:**
Bước 1 : Chạy thử chương trình trên NachOS bằng lệnh  ./userprog/nachos –rs 1023 –x ./test/scheduler rồi Enter

**PROJECT 3:**
Bước 1 : cd vào /project3

Bước 2 : Tiếp tục gõ make để tiến hành tạo file module.ko từ file module.c

Bước 3 : Tiếp tục gõ sudo insmod mymodule.ko để thêm module vào kernel

Bước 4 : Tiếp tục gõ sudo cat /dev/chardev0 để in số ngẫu nhiên ra terminal
