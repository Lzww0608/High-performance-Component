
# 项目说明
该项目主要测试protobuf序列化和反序列的性能

# 编译
在当前工程下先生成.proto文件对应的.cc和.h文件
```
protoc -I=./ --cpp_out=./  person.proto
```

然后
```
mkdir build
cd build 
cmake ..
make
```
得到执行文件person