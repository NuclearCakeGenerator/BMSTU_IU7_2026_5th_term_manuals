# Как делать 2 лабу по архам.

# Подготовка

[Титульник](https://t.me/c/2204134426/84827/94491)

В [чате](https://t.me/c/3847488940/26) с Поповым выслали полезные материалы, кратко изложу их здесь.

1. По этим ссылкам разположены фрагменты образа диска для вирутальной машины с установленным софтом и файлами. Суммарный размер скачивания **5Gb**. После разахивирования получится образ диска который займет еще **20Gb**.
   https://mail.bmstu.ru/~alexpopov@bmstu.ru/vm/evm_lab_vm.7z.001
   https://mail.bmstu.ru/~alexpopov@bmstu.ru/vm/evm_lab_vm.7z.002
   https://mail.bmstu.ru/~alexpopov@bmstu.ru/vm/evm_lab_vm.7z.003

   Разархивируется **одной** командой:
   ```bash
   7z x evm_lab_vm.7z.001
   ```

2. Для запуска Попов предложил скрипт:

    `run.sh`
   ```bash
   qemu-system-x86_64   -enable-kvm   -cpu host   -m 4G   -drive file=riscv1.qcow2,format=qcow2,if=virtio   -vga qxl   -spice port=5900,addr=127.0.0.1,disable-ticketing=on,disable-copy-paste=off   -device virtio-serial-pci   -device virtserialport,chardev=spicechannel0,name=com.redhat.spice.0   -chardev spicevmc,id=spicechannel0,name=vdagent   -netdev user,id=internet   -device virtio-net-pci,mac=50:54:00:00:00:42,netdev=internet,id=internet-dev   -device virtserialport,chardev=charchannel1,id=channel1,name=org.spice-space.webdav.0   -chardev spiceport,name=org.spice-space.webdav.0,id=charchannel1   -device qemu-xhci,id=usb
   ```

   У меня он вызывал постоянные вылеты, так что я сам использую такой:

    `run.sh`
   ```bash
   #!/bin/bash
   qemu-system-x86_64 \
   -enable-kvm \
   -cpu host \
   -smp cores=4,threads=1 \
   -m 8G \
   -device virtio-vga \
   -display none \
   -spice port=5900,addr=127.0.0.1,disable-ticketing=on,disable-copy-paste=off \
   -drive file=riscv1.qcow2,format=qcow2,if=virtio \
   -device virtio-serial-pci \
   -device virtserialport,chardev=spicechannel0,name=com.redhat.spice.0 \
   -chardev spicevmc,id=spicechannel0,name=vdagent \
   -netdev user,id=internet \
   -device virtio-net-pci,mac=50:54:00:00:00:42,netdev=internet,id=internet-dev \
   -device virtserialport,chardev=charchannel1,id=channel1,name=org.spice-space.webdav.0 \
   -chardev spiceport,name=org.spice-space.webdav.0,id=charchannel1 \
   -device qemu-xhci,id=usb
   ```

    Скрипты запукают виртуальную машину, а подключиться к ее "монитору" можно командой:

    `viewer.sh`
    ```bash
    remote-viewer --spice-shared-dir=~/ spice://localhost:5900 --spice-usbredir-auto-redirect-filter="-1,-1,-1,-1,0"
    ```

3. Методичка по работе с софтом достаточно детально описана в [gitlab](https://gitlab.bmstu.ru/iu6-hardware-section/riscv-lab/-/blob/main/main.adoc). 

От себя добавлю:
1. внутри VM для запуска `Questa Sim` вызывается специальный скрипт `run.sh`, но в нем забыли указать имя `.hex` файла изучаемой прошивки - допиши вручную.
2. В последнем задании надо представить две трассировки в виде таблиц, и судя по всему сделать их вручную по диаграммам сигналов. Этот процесс очень кропотливый т.к. никаких экспортов и конвертеров от кафедры не предоставили.

Однако, конвертер есть все же [есть](https://github.com/val3rkq/trace-riscv-excel-gen).

В полученной таблице не забудь заменить коды команд на их читаемое представление (придется вручную поиском с заменой).