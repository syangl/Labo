#on x86 by removing ‘qemu-aarch64’
qemu-aarch64 ./build/benchmark --keys_file=resources/sample_keys.bin --keys_file_type=binary --init_num_keys=500 --total_num_keys=1000 --batch_size=1000 --insert_frac=0.5 --print_batch_stats
