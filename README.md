## ZKP ChaumPedersenプロトコルの実装
- ZKP ChaumPedersen
https://muens.io/chaum-pedersen-protocol/
- テストに使用したデータ  
https://datatracker.ietf.org/doc/html/rfc5114
- grpcのチュートリアル
https://grpc.io/docs/languages/cpp/basics/

## 使用ライブラリ
- boost-multiprecision：大きな数値を扱う
- gtest：単体テスト
- grpc
- protobuf

## 手順
C++20  
vcpkgで依存管理を行っている。  

```bash
# ソースビルド
$ make build-exe
```

```bash
# grpcの生成
# CMakeListsに定義を記載し、cmake/build以下に生成
# target_include_directoriesによってルートからも参照できるように設定している
$ make build-grpc
``` 
