# JSON库

git clone https://github.com/DaveGamble/cJSON.git

git clone https://github.com/open-source-parsers/jsoncpp.git

git clone https://gitee.com/mirrors/RapidJSON.git

除`RapidJSON`外已经将文件集成到测试项目里面，不用再重新编译库文件。

# 性能对比测试
**测试机器:**

处理器：Intel(R) Core(TM) i7-8750U CPU @ 2.20GHz

内存：16G

系统：虚拟机ubuntu 18.04 .1LTS

## 1 测试10万次序列化

|库|默认|-O1|-O2|序列化后字节 |
|-|-|-|-|-|
|cJSON|408ms|422ms|313ms|297|
|jsoncpp|4261ms|716ms|596ms|255|
|rapidjson|605ms|75ms|65ms|239|

## 2 测试10万次反序列化

|库|默认|-O1|-O2|
|-|-|-|-|
|cJSON|250ms|244ms|210ms|
|jsoncpp|3061ms|709ms|478ms|
|rapidjson|1066ms|98ms|93ms|

如果开启优化-O1 / -O2 级别, `rapidjson`和`jsoncpp`提升明显, `cjson`提升不明显。
