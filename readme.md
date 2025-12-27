<div align="center">

# 🚀 NovaLink VPN Evolution: The Stealth Update

![Version](https://img.shields.io/badge/Version-3.0.0--beta-blueviolet?style=for-the-badge)
![C++](https://img.shields.io/badge/C++-20-blue?style=for-the-badge&logo=c%2B%2B)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)
![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Android-lightgrey?style=for-the-badge)

**NovaLink** — это высокопроизводительный стелс-VPN на C++, разработанный для обеспечения максимальной анонимности и обхода продвинутых систем блокировок (DPI).

[Описание](#-о-проекте) • [Стек технологий](#-технологический-стек) • [Сравнение](#-roadmap-эволюции) • [Установка](#-установка-и-сборка)

---

</div>

## 🌟 О проекте (v3.0)
Версия 3.0 полностью пересмотрела архитектуру проекта. Теперь это не просто туннель, а защищенная экосистема:

* **VLESS + Reality-like Obfuscation:** Трафик маскируется под обычный HTTPS (TLS 1.3), что делает его невидимым для DPI.
* **High-Load Epoll Core:** Поддержка тысяч соединений благодаря событийной модели `epoll`.
* **UUID Auth:** Строгая проверка каждого пакета по уникальному идентификатору.
* **Zero-Config:** Клиент сам получает настройки через встроенный REST API.

---

## 🛠 Технологический стек
<p align="left">
  <img src="https://skillicons.dev/icons?i=cpp,linux,cmake,sqlite,githubactions,network" />
</p>

* **Core:** C++20 (Standard Library)
* **Networking:** Linux TUN/TAP, Sockets, Epoll
* **Security:** OpenSSL (TLS 1.3, AES-256-GCM)
* **API & Data:** `cpp-httplib`, `nlohmann/json`, `libcurl`, `SQLite3`

---

## 📈 Roadmap эволюции
<table width="100%">
  <thead>
    <tr>
      <th width="33%">🔴 v1.0 (PoC)</th>
      <th width="33%">🟡 v2.0 (Secure)</th>
      <th width="33%">🟢 v3.0 (Stealth)</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td valign="top">
        <ul>
          <li>UDP Transport</li>
          <li>🔓 No Security</li>
          <li>Hardcoded Config</li>
        </ul>
      </td>
      <td valign="top">
        <ul>
          <li>TCP + TLS 1.3</li>
          <li>🛡 <b>AES-256-GCM</b></li>
          <li>SQLite3 Manager</li>
        </ul>
      </td>
      <td valign="top">
        <ul>
          <li><b>VLESS + Reality</b></li>
          <li><b>High-Load Epoll</b></li>
          <li><b>REST API Sync</b></li>
        </ul>
      </td>
    </tr>
  </tbody>
</table>

---

## 📥 Установка и сборка

### 1. Требования (Зависимости)
Для компиляции в **Kali Linux** или **Ubuntu** выполните:

```bash
sudo apt update && sudo apt install -y \
    build-essential \
    cmake \
    libssl-dev \
    libsqlite3-dev \
    libcurl4-openssl-dev
```
### 2. Сборка проекта


```Bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

🚀 Быстрый запуск
Сервер (VPS) Сгенерируйте SSL-сертификаты:
```Bash
openssl req -x509 -newkey rsa:4096 -keyout server.key -out server.crt -days 365 -nodes
```
Запустите сервер (нужны права root для TUN):

```Bash
    sudo ./novalink_server
```

#### Сервер запустит VPN-ядро на порту 443 и Manager-API на порту 8080.
Клиент

Укажите IP вашего сервера в client/main.cpp (в функции fetch_vpn_config).
Соберите и запустите:
```Bash
    sudo ./novalink_client
```

🏗 Архитектура системы

    Manager-API (C++): Отдает JSON-конфиг с актуальными данными.

    Obfuscation Layer: Добавляет случайные байты к каждому пакету VLESS.

    TUN Interface: Перехватывает системный трафик и направляет его в шифрованный канал.

    Epoll Engine: Эффективно мультиплексирует потоки данных между клиентами и интернетом.