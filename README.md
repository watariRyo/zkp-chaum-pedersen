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
### ビルド
vcpkgで依存管理を行っている。  

```bash
# grpc生成と実行ファイル作成
# CMakeListsに定義を記載し、build以下に生成
# target_include_directoriesによってルートからも参照できるように設定する
$ make build-exe
``` 

### 実行
build以下に、  
```
./build/zkp_server
./build/zkp_client
./build/zkp_test
```
ができる。

`./build/zkp_server`を実行し、別ターミナルで`./build/zkp_client`を実行する形になる。  
コマンドは以下の通り。
- ユーザー登録  
`./build/zkp_client register {user}`
- ログイン  
`./build/zkp_client login {user} {secret}`  
※secretはユーザー登録により出力されたsecretを利用する
