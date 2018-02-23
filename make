#!/usr/bin/env ruby

require 'colorize'

mod = "hashfs"
dev = "/dev/sdb"
mntp = "/mnt/storage"

mku = 'cd util && make clean && make'

tests = <<CMDS
    touch /mnt/storage/nmnm
    touch /mnt/storage/abcd
    touch /mnt/storage/xyzk
    umount /mnt/storage
    sudo mount -t #{mod} #{dev} #{mntp}
    ls /mnt/storage/nmnm
    ls /mnt/storage/abcd
    ls /mnt/storage/xyzk

    ls #{mntp}
    touch #{mntp}/x7x7x7
    ls #{mntp}/x7x7x7

    sudo umount #{dev}

    sudo mount -t #{mod} #{dev} #{mntp}
    ls #{mntp}/x7x7x7

    ./util/debugfs -m #{mntp}
    ./util/debugfs -z #{mntp}

    mkdir /tmp/xx
    ./util/debugfs -m /tmp/xx
    ./util/debugfs -z /tmp/xx
    rm -rf /tmp/xx
CMDS
tests = ''

default = <<CMDS
    cd kmod && make clean && make
    #{mku}

    [ -z "`grep #{dev} /proc/mounts`" ] || sudo umount #{dev}
    [ -z "`grep #{mod} /proc/modules`" ] || sudo rmmod #{mod}

    # ./script/zerofy_hd

    ./util/mkfs #{dev} > /dev/null

    # ./script/clear_pcache
    # ./script/dump_hd

    sudo insmod kmod/#{mod}.ko
    sudo mount -t #{mod} #{dev} #{mntp}

    #{tests}
CMDS

cmds = {
    "u" => mku
}[ARGV[0]]

cmds ||= default 

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
            .map{|l| l.gsub /\s*#.*$/, '' }
            .reject(&:empty?)
            .reject(&:nil?)
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

