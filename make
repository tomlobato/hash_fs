#!/usr/bin/env ruby

require 'colorize'

mod = "hashfs"
dev = "/dev/sdb"
mntp = "/mnt"

cmds = <<CMDS

    [ -z "`grep -v #{dev} /proc/mounts`" ] || sudo umount #{dev}
    [ -z "`grep -v #{mod} /proc/modules`" ] || sudo rmmod #{mod}

    cd kmod && make clean && make
    cd util && make clean && make

    sudo insmod kmod/#{mod}.ko
    sudo mount -t #{mod} #{dev} #{mntp}

    ls #{mntp}

CMDS

# maker

class Maker
    def make cmds
        splitc(cmds).each do |cmd|
            run cmd, die: true
        end
    end

    private

    def splitc cmds
        cmds
            .split(/\n/)
            .reject(&:empty?)
            .reject(&:nil?)
            .compact
            .map(&:strip)
    end

    def run cmd, is_remote: false, die: false, vain: true
        puts cmd.cyan
        # print "#{is_remote ? 'remote' : 'local'}: #{cmd} "

        output = if is_remote
            `#{remote}"#{cmd}" 2>&1`
        else
            `#{cmd} 2>&1`
        end

        if $?.to_i == 0
            puts output if vain && !output.empty?
            # puts 'success'.green
            true
        else
            puts output.red if !output.empty?
            # puts 'error'.red
            exit if die
            false
        end
    end
    
    def section text
        puts "\n-----> #{text}".cyan
    end
end
Maker.new.make cmds

