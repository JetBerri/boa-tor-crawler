#!/usr/bin/env python3
# generate_queries.py

# generate queries in config/queries.txt

import random
import os
from itertools import product

# words by categories

BASE_CATEGORIES = {
    "darknet": [
        "tor hidden service", "darknet forum", "onion site", "deep web index",
        "anonymous chat", "dark market", "osint dark web", "onion mirror",
        "hidden wiki", "dark web leaks", "deep web search", "darknet news",
        "darkweb intelligence", "anonymity tools", "onion scraper", "onion list",
        "hidden service directory", "onion search", "onion archive",
         "hitman", "murder", "weapon", "guns", "explosive", "nude",
        "assassination", "snuff", "fetish", "terror", "trafficking"
    ],
    "privacy": [
        "vpn comparison", "proxy list", "tor relay setup", "pgp key generator",
        "email encryption", "metadata cleaner", "browser fingerprinting",
        "password manager", "anonymous email", "secure messaging",
        "privacy audit", "decentralized identity", "privacy tools"
    ],
    "security": [
        "linux server security", "cybersecurity research", "penetration testing",
        "reverse engineering", "ethical hacking", "exploit development",
        "malware analysis", "incident response", "forensics toolkit",
        "vulnerability scanner", "web app security", "bug bounty"
    ],
    "crypto": [
        "bitcoin mixer", "monero wallet", "cold storage", "defi exchange",
        "blockchain explorer", "coinjoin", "crypto privacy", "hardware wallet",
        "wallet recovery", "crypto transaction analysis", "ledger nano"
    ],
    "leaks": [
        "database leak", "credentials dump", "pastebin leaks", "ransomware dump",
        "source code leak", "exposed s3 bucket", "breached data market",
        "password dump", "data breach tracker"
    ],
    "marketplaces": [
        "onion marketplace", "anonymous vendor", "dark web shop", "escrow service",
        "digital goods market", "forum marketplace", "vendor review"
    ],
    "research": [
        "artificial intelligence", "machine learning", "data mining", "deep learning",
        "nlp research", "bioinformatics", "quantum computing", "open science",
        "scientific journals", "public datasets"
    ],
    "general": [
        "science news", "technology startups", "space exploration", "philosophy",
        "education access", "climate change", "renewable energy", "internet archives",
        "digital culture", "art and design", "health research", "medical AI"
    ],
}

# mods to combine

PREFIXES = [
    "", "how to ", "best ", "top ", "buy ", "download ", "install ",
    "guide ", "tutorial ", "list ", "index of ", "latest ", "2025 ", "2024 "
]

SUFFIXES = [
    "", " online", " forum", " tutorial", " guide", " news", " research",
    " directory", " list", " mirror", " archive", " review", " site"
]

ADDITIONAL_TERMS = [
    "manual", "overview", "comparison", "setup", "configuration", "integration",
    "tools", "tips", "tricks", "best practices", "case study", "whitepaper",
    "api", "sdk", "cli"
]

# illegal queries

ILLEGAL = [
    "rape", "sexual assault", "molest", "abuse", "sexual abuse",
    "sex crime", "sex trafficking", "exploit", "sex slave", "forced sex",
    "non-consensual", "incest", "bestiality", "zoophilia",
    "child porn", "child pornography", "cp", "pedo", "pedophile",
    "pedophilia", "underage", "minor sex", "child exploitation",
    "baby porn", "infant porn", "teen porn", "lolita", "pthc", "r@ygold",
    "boylover", "girllover", "csem", "csa", "preteen", "young nude",
    "grooming", "entrapment", "luring", "minor contact", "predator",
    "child grooming", "sexual grooming", "underage chat",
    "forced", "nonconsensual", "revenge porn", "blackmail sex",
    "sex torture", "snuff", "coerced", "abduction", "sex slavery",
    "darkweb cp", "trade cp", "buy cp", "sell cp", "exchange cp",
    "hidden wiki cp", "deepweb cp", "pedo onion", "child abuse material",
    "cam child", "underage cam", "abuse photo", "abuse video"
]

SAFE_BLOCKLIST = set(w.lower() for w in ILLEGAL)

def generate_queries(target_min=1000, shuffle=True):
    os.makedirs("config", exist_ok=True)

    queries = set()

    for cat, terms in BASE_CATEGORIES.items():
        for term in terms:
            for pre in PREFIXES:
                for suf in SUFFIXES:
                    # combinations
                    q = f"{pre}{term}{suf}".strip()
                    q = " ".join(q.split())  # normalize spaces
                    q_lower = q.lower()
                    if any(b in q_lower for b in SAFE_BLOCKLIST):
                        continue
                    queries.add(q)

    all_base = [t for terms in BASE_CATEGORIES.values() for t in terms]
    for a, b in product(all_base, repeat=2):
        if a == b:
            continue
        q = f"{a} {b}"
        q_lower = q.lower()
        if any(bad in q_lower for bad in SAFE_BLOCKLIST):
            continue
        queries.add(q)

    tech_terms = [
        "rpc", "node", "exploit", "scanner", "crawler", "scraper",
        "dataset", "api", "dashboard", "monitor", "plugin", "extension"
    ] + ADDITIONAL_TERMS

    for base in all_base:
        for t in tech_terms:
            q = f"{base} {t}"
            if any(bad in q.lower() for bad in SAFE_BLOCKLIST):
                continue
            queries.add(q)

    years = ["2025", "2024", "2023"]
    for q in list(queries)[:2000]:
        for y in years:
            newq = f"{q} {y}"
            if any(bad in newq.lower() for bad in SAFE_BLOCKLIST):
                continue
            queries.add(newq)

    # random variants

    words_pool = list({w for q in queries for w in q.split() if len(w) > 2})
    random.seed(42)
    for _ in range(3000):
        a = random.choice(words_pool)
        b = random.choice(words_pool)
        c = random.choice(words_pool)
        q = f"{a} {b} {c}"
        if any(bad in q.lower() for bad in SAFE_BLOCKLIST):
            continue
        queries.add(q)

    queries = list(queries)
    if shuffle:
        random.shuffle(queries)

    if len(queries) < target_min:
        # duplicate num sufix
        idx = 0
        while len(queries) < target_min:
            queries.append(queries[idx % len(queries)] + f" extra {idx}")
            idx += 1

    # write to file

    qfile = "config/queries.txt"
    with open(qfile, "w", encoding="utf-8") as f:
        for q in queries:
            f.write(q + "\n")

    print(f"✅ Generated {len(queries)} queries -> {qfile}")
    print(f"✅ Wrote {len(ILLEGAL)} illegal/blocked terms -> {illegal_file}")
    return queries

if __name__ == "__main__":
    # generate as many queries as you want, min value
    generate_queries(target_min=1500)
