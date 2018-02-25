#!/usr/bin/env ruby

require 'colorize'

mod = "hashfs"
dev = "/dev/sdb"
mntp = "/mnt/storage"

cmd_map = {
    tests: "
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
    ",
    re_fmt_mount: %Q{
        [ -z "`grep #{dev} /proc/mounts`" ] || sudo umount #{dev}
        [ -z "`grep #{mod} /proc/modules`" ] || sudo rmmod #{mod}

        # ./helpers/zerofy_hd

        ./util/mkfs #{dev} > /dev/null

        # ./helpers/clear_pcache
        # ./helpers/dump_hd
        # hexdump -s 976 -n 2500000 -C $dev

        sudo insmod kmod/#{mod}.ko
        sudo mount -t #{mod} #{dev} #{mntp}
    },
    utils_mk: 'cd util && make',
    kmod_mk: 'cd kmod && make',
    clean: "
        cd util && make clean
        cd kmod && make clean
    ",
    default: "
        :kmod_mk
        :utils_mk
        :re_fmt_mount
        # :tests
    "
}

# maker

class Maker
    def initialize cmd_map
        @cmd_map = cmd_map
        @cmds = proc_cmd_map
    end

    def proc_cmd_map
        @cmd_map.keys.each do |k| 
            s_key = k.to_s[0].to_sym
            if @cmd_map[s_key]
                raise "Short key #{s_key} repeated. Aborting."
            end
            @cmd_map[s_key] = @cmd_map[k]
        end
    end

    def build_cmds cmd_alias
        cmd_alias = cmd_alias ? 
            cmd_alias.to_sym : 
            :default
        cmds = @cmd_map[cmd_alias] || raise("unknown command '#{cmd_alias}'")
        splitc cmds
    end

    def make cmd_alias
        build_cmds(cmd_alias).each do |cmd|
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
            .map{|c| 
                if c =~ /^:(.*)$/
                    _c = $1.to_sym
                    @cmd_map[_c] ?
                        splitc(@cmd_map[_c]) :
                        raise("uknown subcommand '#{@cmd_map[_c]}'")
                else
                    c
                end
            }
            .flatten
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

Maker.new(cmd_map).make ARGV[0]

