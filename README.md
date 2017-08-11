# MQShrinkWrap
メタセコイア４用ShrinkWrap + Gravity互換モード。多角形対応

## ４種類の吸着モード
 - 通常のShrinkWrap
   - 一番近い面の一番近い面上へ頂点を移動
 - mqdlさんのGravity.dll互換モード
   - X軸のみ移動
   - Y軸のみ移動
   - Z軸のみ移動
   - カメラ方向のみ移動
 
 ## 使い方
  - 移動させたい頂点を選択します
  - メニュー -> 選択部処理 -> ShrinkWrap
  - 吸着先のオブジェクト & 吸着モードを選んでOK押す

## ダウンロード (64ビット版のみ）
 - https://github.com/devil-tamachan/MQShrinkWrap/releases
 - 32ビット版が欲しい人は自分でコンパイルしてください

## インストール方法
 - MQShrinkWrap.dll, ShrinkWrap.exe, CGAL-vc100-mt-4.9.1.dl_ の３ファイルを　Metasequoia4_x64\Plugins\Select　フォルダへコピー、貼り付け
 
## 詳しい動作解説
 - ポリゴンの裏表関係なく一番近い面上に吸着します。
 - Gravity互換モードは正方向と逆方向の近い方へ吸着します。例えば "カメラ方向のみ移動" の場合、頂点から画面奥、頂点から画面手前の近い方へ吸着します。"Y軸のみ移動" の場合、頂点から直上、直下の近い面へ吸着します

## コンパイル方法
 - VCインストール
 - CGAL4.9.1をインストール、CMakeでビルド (4.9.1までVC2010対応)
 - Boostインストール
 - CMakeでコンパイル

## メタセコイア４ プラグインリンク集
 - https://docs.google.com/spreadsheets/d/1HNXBzK-aAXoJ_Dp4ijxiaHkrvYuYB7sx4kyf-tdV0Hk/edit#gid=0
