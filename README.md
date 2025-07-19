# alt-ime-arch

## 概要

**alt-ime-arch**は、Arch LinuxのHyprland環境で、Fcitx5を用いた日本語入力切り替えを簡単にするためのソフトウェアです。  
キーボードのAltキー（左/右）単体押しにより、IME（Fcitx5）のON/OFF切り替えを実現します。  
Altキー＋他のキーの組み合わせは通常通り動作します。

## 主な機能

- 左Altキー単押しでIME（Fcitx5）をOFF
- 右Altキー単押しでIME（Fcitx5）をON
- Altキー＋他のキーは通常の修飾キーとして動作
- キーボードのイベントデバイスを引数で指定して動作

## 使い方

1. **ビルド方法**  
   必要なC++17環境で以下を実行してください。

   ```bash
   make
   ```

2. **実行方法**  
   キーボードのイベントデバイス（例: `/dev/input/event0`）を指定して起動します。

   ```bash
   sudo ./build/alt-ime-arch /dev/input/event0
   ```

3. **キーボードデバイスの調べ方**  
   - `sudo libinput list-devices`
   - または `/proc/bus/input/devices` を参照

4. **パーミッション設定例**  
   権限が不足する場合、以下を実行してください。

   ```bash
   sudo usermod -a -G input $USER
   sudo tee /etc/udev/rules.d/99-uinput.rules << EOF
   KERNEL=="uinput", GROUP="input", MODE="0660"
   EOF
   sudo udevadm control --reload-rules && sudo udevadm trigger
   # ログアウト・ログイン
   ```

## 実装概要

- `/dev/uinput` を利用した仮想キーボードデバイスを作成し、Altキーイベントを監視・転送
- Altキー単押しと組み合わせ押しを判定し、IME切り替えコマンド（`fcitx5-remote -o`/`fcitx5-remote -c`）を実行
- 標準出力で状態を表示

## ライセンス

MIT License  
Copyright (c) 2025 er-GOD-ic

## 開発者向け情報

- ソースコードは`src/main.cpp`に実装
- ビルド手順は`Makefile`に記載
- Arch Linux + Hyprland + Fcitx5環境向け

## 注意事項

- root権限または/dev/inputデバイスへのアクセス権が必要です。
- IME切り替えには`fcitx5-remote`コマンドが必要です。

## 作者

er-GOD-ic
