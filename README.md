Bước 1 : Cài đặt Linux trên máy ảo

Bước 2 : Tải và giải nén file projecthdh.rar

Bước 3 : Vào thư mục projecthdh/gnu-decstation-ultrix/decstation-ultrix/2.95.3/  Thiết lập thuộc tính excutable ở mọi người dùng đối với tất cả các file có trong thư mục (as, cc1, cpp0, gcc, ld).

Bước 4 : Sửa dòng “MAKE = gmake” thành MAKE = make và lưu lại.

Bước 5 : Mở Terminal. Gõ lệnh cd ~/projecthdh
/nachos-3.4/code rồi Enter.

Bước 6 : Tiếp tục gõ make để tiến hành cài đặt NachOS 

Bước 7 : Chạy thử chương trình trên NachOS bằng lệnh : ./userprog/nachos –rs 1023 –x ./test/halt rồi Enter
