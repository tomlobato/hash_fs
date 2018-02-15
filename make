#!/usr/bin/env ruby

require 'colorize'

mod = "hashfs"
dev = "/dev/sdb"
mntp = "/mnt/storage"

cmds = <<CMDS

    cd kmod && make clean && make
    cd util && make clean && make

    [ -z "`grep #{dev} /proc/mounts`" ] || sudo umount #{dev}
    [ -z "`grep #{mod} /proc/modules`" ] || sudo rmmod #{mod}

    # ./util/mkfs #{dev} > /dev/null

    sudo insmod kmod/#{mod}.ko
    sudo mount -t #{mod} #{dev} #{mntp}

    # touch /mnt/storage/1234
    # touch /mnt/storage/abcd
    # touch /mnt/storage/xyzk
    # umount /mnt/storage
    # sudo mount -t #{mod} #{dev} #{mntp}
    # ls /mnt/storage/1234
    # ls /mnt/storage/abcd
    # ls /mnt/storage/xyzk

    # ls #{mntp}
    # touch #{mntp}/x7x7x7
    # ls #{mntp}/x7x7x7

    # sudo umount #{dev}

    # sudo mount -t #{mod} #{dev} #{mntp}
    # ls #{mntp}/x7x7x7

CMDS

# maker

class Maker
    def make cmds
        splitc(cmds).each do |cmd|
            run cmd
        end
    end

    private

    def splitc cmds
        cmds
            .split(/\n/)
            .map(&:strip)
            .reject(&:empty?)
            .reject(&:nil?)
            .reject{|l| l =~ /^s*#/ }
            .compact
    end

    def run cmd, is_remote: false, die: true, vain: true
        puts cmd.cyan
        system cmd
        if $?.to_i != 0
            puts "error".red
            exit if die
        end
    end
end

Maker.new.make cmds

