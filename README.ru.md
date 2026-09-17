# GLSSH — форк OpenSSH

Этот репозиторий — форк [OpenSSH](https://www.openssh.com) ([openssh-portable](https://github.com/openssh/openssh-portable)), основан на версии **OpenSSH_10.2** (см. [version.h](version.h)).

Добавленная функциональность описана ниже. Всё остальное — стандартный portable OpenSSH: сборка, документация и лицензия не меняются (см. [LICENCE](LICENCE) и [CREDITS](CREDITS) — этот форк распространяется под той же лицензией, что и оригинальный OpenSSH).

---

### Описание

GLSSH — расширение OpenSSH, которое перехватывает запросы аутентификации (пароль, OTP, passphrase) и автоматически подставляет ответ на основе настроенных событий.

### Возможности

* Перехват любого запроса ввода (пароль, OTP, PIN, passphrase)
* Сопоставление запроса по регулярному выражению (POSIX regex)
* Подстановка статического текста
* Выполнение команд и скриптов для динамической генерации ответов
* Несколько событий с контролем количества использований

### Дополнительно

* Возможно дополнять и переопределять уже указанные стандартные параметры

### Синтаксис конфигурации

```
GLSSHAddRequestEvent "regex" "data" [count]
```

| Параметр | Описание |
| --- | --- |
| `regex` | Регулярное выражение для поиска в строке запроса |
| `data` | Текст для подстановки или команда (`$(скрипт или путь к скрипту)`) |
| `count` | Количество раз, которое событие может быть использовано (по умолчанию 1) |

### Примеры

```
# ~/.ssh/config

# Подстановка статического пароля
GLSSHAddRequestEvent "[Pp]assword" "mysecret"

# Выполнение команды для OTP
GLSSHAddRequestEvent "OTP:" "$(oathtool --totp -b SECRET)"

# Сложный regex
GLSSHAddRequestEvent "^\(user@host\).*Password: $" "$(pass show host/password)"

# Множественные события (выполняются по порядку)
GLSSHAddRequestEvent "Username:" "admin"
GLSSHAddRequestEvent "Password:" "$(/usr/local/bin/get-pass.sh)"
GLSSHAddRequestEvent "OTP:" "$(oathtool --totp -b pass show totp/secret)"

# Переопределение значений стандартных параметров
Host test-*
    User user1

Host test-2
    User user2

# Дополнение значений стандартных параметров
Host test-*
    User user

Host test-2
    User +2
```

### Контроль количества использований

```
# Событие можно использовать 3 раза
GLSSHAddRequestEvent "Password:" "mysecret" 3

# Повторные попытки: событие сработает 5 раз
GLSSHAddRequestEvent "[P-p]assword" "temp123" 5
```

### Позиционная подстановка для keyboard-interactive

Когда сервер отправляет несколько запросов подряд с одинаковым или пустым текстом, можно полагаться на порядок событий и счётчик использования. Для этого используется regex `.*`, который соответствует любому запросу:

```
# Первый запрос (любой) → username
GLSSHAddRequestEvent ".*" "admin"
# Второй запрос → пароль
GLSSHAddRequestEvent ".*" "$(pass show host/password)"
# Третий запрос → OTP
GLSSHAddRequestEvent ".*" "$(oathtool --totp -b SECRET)"
```

События применяются по очереди: каждое срабатывает ровно один раз (или указанное число раз), после чего переходит к следующему. Это позволяет автоматизировать диалоги, где текст запросов неразличим или отсутствует.

### Как это работает

1. Сервер запрашивает аутентификацию (например, `"Password: "`)
2. Клиент перехватывает запрос в `read_passphrase()`
3. Проверяет все события в порядке добавления
4. Находит первое событие с regex, совпадающим с запросом
5. Подставляет `data` как ответ (если `data` начинается с `$` — выполняет команду)
6. Пользователь получает доступ без ручного ввода

### Отладка

```
ssh -v host 2>&1 | grep GLSSH
```

### Сборка

Файлы из `source` перекидываем с заменой в исходный код OpenSSH.
Выполняем сборку проекта через `make`.

### Лицензия

GLSSH является расширением OpenSSH и распространяется под той же лицензией.

---

# Portable OpenSSH

[![C/C++ CI](../../actions/workflows/c-cpp.yml/badge.svg)](../../actions/workflows/c-cpp.yml)
[![VM CI](../../actions/workflows/vm.yml/badge.svg)](../../actions/workflows/vm.yml)
[![C/C++ CI self-hosted](https://github.com/openssh/openssh-portable-selfhosted/actions/workflows/selfhosted.yml/badge.svg)](https://github.com/openssh/openssh-portable-selfhosted/actions/workflows/selfhosted.yml)
[![CIFuzz](../../actions/workflows/cifuzz.yml/badge.svg)](../../actions/workflows/cifuzz.yml)
[![Fuzzing Status](https://oss-fuzz-build-logs.storage.googleapis.com/badges/openssh.svg)](https://issues.oss-fuzz.com/issues?q="Project:+openssh"+is:open)
[![Coverity Status](https://scan.coverity.com/projects/21341/badge.svg)](https://scan.coverity.com/projects/openssh-portable)

OpenSSH — это полная реализация протокола SSH (версия 2) для безопасного удалённого входа, выполнения команд и передачи файлов. Он включает клиент ``ssh`` и сервер ``sshd``, утилиты передачи файлов ``scp`` и ``sftp``, а также инструменты для генерации ключей (``ssh-keygen``), хранения ключей во время выполнения (``ssh-agent``) и ряд вспомогательных программ.

Это перенос (порт) [OpenSSH](https://openssh.com) от OpenBSD на большинство Unix-подобных операционных систем, включая Linux, OS X и Cygwin. Portable OpenSSH реализует полифиллы для API OpenBSD, недоступных в других системах, добавляет sandboxing для sshd на большем числе операционных систем и включает поддержку нативной для ОС аутентификации и аудита (например, через PAM).

**Примечание:** этот форк (GLSSH) добавляет расширение GLSSH, описанное выше; всё, что ниже, — неизменённая документация оригинального проекта.

## Документация

Официальная документация OpenSSH — это man-страницы для каждого инструмента:

* [ssh(1)](https://man.openbsd.org/ssh.1)
* [sshd(8)](https://man.openbsd.org/sshd.8)
* [ssh-keygen(1)](https://man.openbsd.org/ssh-keygen.1)
* [ssh-agent(1)](https://man.openbsd.org/ssh-agent.1)
* [scp(1)](https://man.openbsd.org/scp.1)
* [sftp(1)](https://man.openbsd.org/sftp.1)
* [ssh-keyscan(8)](https://man.openbsd.org/ssh-keyscan.8)
* [sftp-server(8)](https://man.openbsd.org/sftp-server.8)

## Стабильные релизы

Архивы стабильных релизов доступны на ряде [зеркал для загрузки](https://www.openssh.com/portable.html#downloads). Большинству пользователей мы рекомендуем использовать стабильный релиз. Ознакомьтесь с [примечаниями к релизу](https://www.openssh.com/releasenotes.html), чтобы узнать подробности о недавних изменениях и возможных несовместимостях.

## Сборка Portable OpenSSH

### Зависимости

Portable OpenSSH собирается с помощью autoconf и make. Требуется работающий компилятор C, стандартная библиотека и заголовочные файлы.

Также может использоваться ``libcrypto`` из [LibreSSL](https://www.libressl.org/), [OpenSSL](https://www.openssl.org), [AWS-LC](https://github.com/aws/aws-lc) или [BoringSSL](https://github.com/google/boringssl). OpenSSH может быть собран без них, но в этом случае итоговые бинарные файлы будут поддерживать лишь подмножество обычно доступных криптографических алгоритмов.

[zlib](https://www.zlib.net/) не обязателен; без него не поддерживается сжатие транспортного уровня.

Поддержка FIDO security token требует [libfido2](https://github.com/Yubico/libfido2) и его зависимостей и будет включена автоматически, если они найдены.

Кроме того, некоторые платформы и опции сборки могут требовать дополнительных зависимостей; подробности для вашей платформы см. в README.platform.

### Сборка релиза

Архивы релизов и релизные ветки в git включают заранее собранную копию скрипта ``configure`` и могут быть собраны следующим образом:

```
tar zxvf openssh-X.YpZ.tar.gz
cd openssh
./configure # [опции]
make && make tests
```

Опции configure описаны в разделе [Настройка параметров сборки](#настройка-параметров-сборки) ниже. Если вы планируете устанавливать OpenSSH в систему, вам, как правило, потребуется указать пути установки.

### Сборка из git

При сборке из ветки git master вам потребуется установленный [autoconf](https://www.gnu.org/software/autoconf/) для сборки скрипта ``configure``. Следующие команды позволяют получить исходный код и собрать portable OpenSSH из git:

```
git clone https://github.com/openssh/openssh-portable # или https://anongit.mindrot.org/openssh.git
cd openssh-portable
autoreconf
./configure
make && make tests
```

### Настройка параметров сборки

Доступно множество опций настройки сборки. Поддерживаются все флаги путей установки Autoconf (например, ``--prefix``), и они, как правило, необходимы, если вы хотите установить OpenSSH.

Полный список доступных флагов можно получить, выполнив ``./configure --help``, но некоторые из наиболее часто используемых описаны ниже. Для некоторых из этих флагов потребуется установка дополнительных библиотек и/или заголовочных файлов.

Флаг | Значение
--- | ---
``--with-pam`` | Включает поддержку [PAM](https://en.wikipedia.org/wiki/Pluggable_authentication_module). Поддерживаются [OpenPAM](https://www.openpam.org/), [Linux PAM](http://www.linux-pam.org/) и Solaris PAM.
``--with-libedit`` | Включает поддержку [libedit](https://www.thrysoee.dk/editline/) для sftp.
``--with-kerberos5`` | Включает поддержку Kerberos/GSSAPI. Поддерживаются реализации Kerberos как [Heimdal](https://www.h5l.org/), так и [MIT](https://web.mit.edu/kerberos/).
``--with-selinux`` | Включает поддержку [SELinux](https://en.wikipedia.org/wiki/Security-Enhanced_Linux).

## Разработка

Разработка Portable OpenSSH обсуждается в [списке рассылки openssh-unix-dev](https://lists.mindrot.org/mailman/listinfo/openssh-unix-dev) ([зеркало архива](https://marc.info/?l=openssh-unix-dev)). Ошибки и запросы новых функций отслеживаются в нашем [Bugzilla](https://bugzilla.mindrot.org/).

## Сообщения об ошибках

О _несвязанных с безопасностью_ ошибках можно сообщить разработчикам через [Bugzilla](https://bugzilla.mindrot.org/) или через указанный выше список рассылки. О проблемах безопасности следует сообщать по адресу [openssh@openssh.com](mailto:openssh.openssh.com).
