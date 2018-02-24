#!/usr/bin/env ruby

require 'colorize'

mod = "hashfs"
dev = "/dev/sdb"
mntp = "/mnt/storage"

mku = 'cd util && make clean && make'

tests = <<CMDS
    touch #{mntp}/nmnm
    touch #{mntp}/abcd
    touch #{mntp}/xyzk
    umount #{mntp}
    sudo mount -t #{mod} #{dev} #{mntp}
    ls #{mntp}/nmnm
    ls #{mntp}/abcd
    ls #{mntp}/xyzk
    rm #{mntp}/nmnm
    rm #{mntp}/abcd
    rm #{mntp}/xyzk

    ls #{mntp}
    touch #{mntp}/x7x7x7
    ls #{mntp}/x7x7x7

    sudo umount #{dev}

    sudo mount -t #{mod} #{dev} #{mntp}
    ls #{mntp}/x7x7x7
    rm #{mntp}/x7x7x7

    bash ./helpers/test.sh

    ./util/debugfs -m #{mntp}
    ./util/debugfs -z #{mntp}
CMDS

default = <<CMDS
    cd kmod && make clean && make
    #{mku}

    [ -z "`grep #{dev} /proc/mounts`" ] || sudo umount #{dev}
    [ -z "`grep #{mod} /proc/modules`" ] || sudo rmmod #{mod}

    # ./helpers/zerofy_hd

    ./util/mkfs #{dev} > /dev/null

    # ./helpers/clear_pcache
    # ./helpers/dump_hd
    # hexdump -s 976 -n 2500000 -C $dev

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

