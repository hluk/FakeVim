#!/bin/bash
# Generate tests for FakeVim.
testfile="test.cpp"
logfile="test.log"
vim="vim"
vimopts=(-X -u NONE)

print_cmd()
{
    printf '%s' "$@" | sed 's/<cr>/\n/gI;s/<esc>//gI'
}

print_status()
{
    {
        if [ $# -eq 0 ]; then
            printf 'data.setText("';
        else
            printf 'KEYS("%s", "' "$1";
        fi

        local first=1
        while read line
        do
            if [[ $first == 1 ]]; then
                first=0
            else
                printf '" N "'
            fi
            printf '%s' "$(sed 's/|/" X "/' <<< "$line")"
        done

        printf '");\n';
    } | sed 's/N "" X/N X/' >> "$logfile"
}

clean_up()
{
    rm -f ".${testfile}"*".swp" "$testfile"* "$logfile"
}

main()
{
    if [ $# -lt 2 ]; then
        printf "Usage: %s [file content] [commands ...]\n" 1>&2
        exit 2
    fi

    clean_up

    contents=$1
    printf "%s" "$contents" > "$testfile"
    shift

    print_status < "$testfile"

    i=0
    {
        echo ':set nocompatible'
        echo ':%s/|//'
        for cmd in "$@"; do
            echo ":redir >> $logfile"

            echo ":normal $cmd"
            #echo ":map Q $cmd"
            #print_cmd 'Q<ESC><ESC>'
            #read 2>&1 > /dev/null

            tmpfile="${testfile}.$((++i))"
            echo ":w $tmpfile"
            echo ":redir >> $tmpfile"
            echo ":echo getpos('.')"
            echo ":redir >>"
        done
        echo 'ZZ'
    } | "$vim" "${vimopts[@]}" -s /dev/stdin "$testfile" &> /dev/null || exit 1

    i=0
    for cmd in "$@"; do
        tmpfile="${testfile}.$((++i))"
        [ -f "$tmpfile" ] || exit 3

        cursor=$(tail -1 "$tmpfile")
        print_cmd ':$-2,$d|call setpos(".", '"$cursor"')<CR>i|<ESC>ZZ' | "$vim" $vimopts -s /dev/stdin "$tmpfile" &> /dev/null || exit 4
        head "$tmpfile" | print_status "$cmd<ESC><ESC>"
    done

    cat "$logfile"

    clean_up
}

main "$@"

