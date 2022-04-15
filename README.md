**PROJECT 1:**

Bước 1 : Cài đặt Linux trên máy ảo

Bước 2 : Tải và giải nén file _projecthdh.rar_

Bước 3 : Vào thư mục _projecthdh/gnu-decstation-ultrix/decstation-ultrix/2.95.3/_  Thiết lập thuộc tính excutable ở mọi người dùng đối với tất cả các file có trong thư mục (as, cc1, cpp0, gcc, ld).

Bước 4 : Sửa dòng _“MAKE = gmake”_ thành _"MAKE = make"_ và lưu lại.

Bước 5 : cd vào _/code_ 

Bước 6 : Tiếp tục gõ make để tiến hành cài đặt NachOS 

Bước 7 : cd vào ./test và gõ make để compile file .c

Bước 8 : Chạy thử chương trình trên NachOS bằng lệnh  _../userprog/nachos -rs 1023 -x ./halt_ 

**PROJECT 2:**

Bước 1 : Làm tương tự PROJECT 1 rồi chạy thử chương trình trên NachOS bằng lệnh  _../userprog/nachos -rs 1023 -x ./scheduler_ 

**PROJECT 3:**

Bước 1 : cd vào _/project3_

Bước 2 : Tiếp tục gõ _make_ để tiến hành tạo file module.ko từ file module.c

Bước 3 : Tiếp tục gõ _sudo insmod mymodule.ko_ để thêm module vào kernel (_sudo rmmod mymodule.ko_ để gỡ module)

Bước 4 : Tiếp tục gõ _sudo cat /dev/chardev0_ để in số ngẫu nhiên ra terminal
