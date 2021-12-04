use rand::Rng;

fn find_tile_pos(board: &str, find: char) -> usize {
    return board.find(find).unwrap();
}

#[test]
fn test_find_tile_pos() {
    assert_eq!(find_tile_pos("123456780", '1'), 0);
    assert_eq!(find_tile_pos("123456780", '3'), 2);
    assert_eq!(find_tile_pos("123456780", '0'), 8);
}

#[test]
#[should_panic]
fn test_find_tile_pos_panic() {
    // panic
    find_tile_pos("123456780", 'a');
}

fn find_empty_pos(board: &str) -> usize {
    return find_tile_pos(board, '0');
}

#[test]
fn test_find_empty_pos() {
    assert_eq!(find_empty_pos("123456780"), 8);
    assert_eq!(find_empty_pos("123056784"), 3);
    assert_eq!(find_empty_pos("023456781"), 0);
}

#[test]
#[should_panic]
fn test_find_empty_pos_panic() {
    // panic
    find_empty_pos("12345678a");
}

fn move_tile(board: &str, from_pos: usize) -> String {
    let mut ret = board.to_string();
    let empty_pos = find_empty_pos(board);
    let c = ret.chars().nth(from_pos).unwrap();
    ret.replace_range(empty_pos..empty_pos + 1, &c.to_string());
    ret.replace_range(from_pos..from_pos + 1, "0");

    return ret;
}

#[test]
fn test_move_tile() {
    assert!(move_tile("123456780", 2).eq("120456783"));
    assert!(move_tile("023456781", 0).eq("023456781"));
}

fn list_movable_tiles(board: &str) -> Vec<usize> {
    let empty_pos = find_empty_pos(board);

    if empty_pos > 3 {
        vec![empty_pos - 3, empty_pos - 1, empty_pos + 1, empty_pos + 3]
    } else if empty_pos > 1 {
        vec![empty_pos - 1, empty_pos + 3, empty_pos + 1]
    } else {
        vec![empty_pos + 1, empty_pos + 3]
    }
    .iter()
    .filter(|&&x| (empty_pos / 3 == x / 3 || empty_pos % 3 == x % 3) && x < 9)
    .map(|&x| x)
    .collect()
}

#[test]
fn test_list_movable_tiles() {
    assert!(list_movable_tiles("123456780").eq(&vec![5, 7]));
    assert!(list_movable_tiles("253107486").eq(&vec![1, 3, 5, 7]));
    assert!(list_movable_tiles("012453867").eq(&vec![1, 3]));
}

fn aligned_board() -> String {
    "123456780".to_string()
}

fn random_board(iteration: u16) -> String {
    let mut board = aligned_board();

    let mut rng = rand::thread_rng();

    for _ in 0..iteration {
        let movable_tiles = list_movable_tiles(&board);
        let tile_to_move = movable_tiles[rng.gen::<usize>() % movable_tiles.len() as usize];
        board = move_tile(&board, tile_to_move);
    }

    board
}

fn dump_board(board: &str) {
    println!(" ------ ");
    let mut chars = board.chars();

    for _ in 0..3 {
        println!(
            "| {} {} {} |",
            chars.next().unwrap(),
            chars.next().unwrap(),
            chars.next().unwrap()
        );
    }

    println!(" ------ ");
}
fn main() {
    let board = random_board(100);
    println!("{}", board);
    dump_board(&board);
}
